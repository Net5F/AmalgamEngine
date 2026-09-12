cmake_minimum_required(VERSION 3.24)

if (DEFINED FORCE)
    message(FATAL_ERROR
        "FORCE is not supported. Manually delete the existing remote TLS "
        "credentials after confirming that they are no longer needed.")
endif()

if (NOT DEFINED PROJECT_DIR OR PROJECT_DIR STREQUAL "")
    message(FATAL_ERROR
        "PROJECT_DIR is required. Example:\n"
        "  cmake -DPROJECT_DIR=/path/to/project "
        "-P /path/to/AmalgamEngine/Scripts/GenerateTlsCredentials.cmake")
endif()

cmake_path(ABSOLUTE_PATH PROJECT_DIR NORMALIZE
           OUTPUT_VARIABLE NORMALIZED_PROJECT_DIR)
if (NOT IS_DIRECTORY "${NORMALIZED_PROJECT_DIR}")
    message(FATAL_ERROR
        "PROJECT_DIR is not a directory: ${NORMALIZED_PROJECT_DIR}")
endif()

set(REMOTE_CREDENTIALS_DIR
    "${NORMALIZED_PROJECT_DIR}/TlsCredentials/Remote")
file(MAKE_DIRECTORY "${REMOTE_CREDENTIALS_DIR}")

file(GLOB EXISTING_PRIVATE_KEYS
    LIST_DIRECTORIES FALSE
    "${REMOTE_CREDENTIALS_DIR}/*.key")
if (EXISTING_PRIVATE_KEYS)
    list(JOIN EXISTING_PRIVATE_KEYS "\n    " EXISTING_PRIVATE_KEY_LIST)
    message(FATAL_ERROR
        "Remote TLS private keys already exist:\n\n"
        "    ${EXISTING_PRIVATE_KEY_LIST}\n\n"
        "To avoid accidentally losing deployed credentials, this script "
        "will not overwrite them. Back up any credentials that are still "
        "needed, then manually delete the .key files and run the script "
        "again.")
endif()

if (DEFINED OPENSSL_EXECUTABLE AND NOT OPENSSL_EXECUTABLE STREQUAL "")
    cmake_path(ABSOLUTE_PATH OPENSSL_EXECUTABLE NORMALIZE
               OUTPUT_VARIABLE OPENSSL_COMMAND)
    if (NOT EXISTS "${OPENSSL_COMMAND}")
        message(FATAL_ERROR
            "OPENSSL_EXECUTABLE was not found: ${OPENSSL_COMMAND}")
    endif()
else()
    set(OPENSSL_HINTS "")
    if (DEFINED OPENSSL_ROOT_DIR AND NOT OPENSSL_ROOT_DIR STREQUAL "")
        list(APPEND OPENSSL_HINTS "${OPENSSL_ROOT_DIR}/bin")
    endif()
    if (DEFINED ENV{OPENSSL_ROOT_DIR}
        AND NOT "$ENV{OPENSSL_ROOT_DIR}" STREQUAL "")
        list(APPEND OPENSSL_HINTS "$ENV{OPENSSL_ROOT_DIR}/bin")
    endif()

    find_program(OPENSSL_COMMAND
        NAMES openssl openssl.exe
        HINTS ${OPENSSL_HINTS})
    if (NOT OPENSSL_COMMAND)
        message(FATAL_ERROR
            "OpenSSL was not found. Add it to PATH or pass "
            "-DOPENSSL_EXECUTABLE=/path/to/openssl.")
    endif()
endif()

execute_process(
    COMMAND "${OPENSSL_COMMAND}" version
    RESULT_VARIABLE OPENSSL_VERSION_RESULT
    OUTPUT_VARIABLE OPENSSL_VERSION
    ERROR_VARIABLE OPENSSL_VERSION_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE)
if (NOT OPENSSL_VERSION_RESULT EQUAL 0)
    message(FATAL_ERROR
        "Failed to run OpenSSL: ${OPENSSL_VERSION_ERROR}")
endif()
if (NOT OPENSSL_VERSION MATCHES "^OpenSSL 3\\.5\\.")
    message(FATAL_ERROR
        "OpenSSL 3.5.x is required, but found: ${OPENSSL_VERSION}")
endif()

set(SERVER_FILE_STEMS
    account-server
    chat-server
    world-server)
set(SERVER_COMMON_NAMES
    "Amalgam Account Server"
    "Amalgam Chat Server"
    "Amalgam World Server")

string(RANDOM LENGTH 12 ALPHABET 0123456789abcdef STAGING_SUFFIX)
set(STAGING_DIR
    "${NORMALIZED_PROJECT_DIR}/TlsCredentials/.staging-${STAGING_SUFFIX}")
file(MAKE_DIRECTORY "${STAGING_DIR}")

function(fail_generation MESSAGE_TEXT)
    file(REMOVE_RECURSE "${STAGING_DIR}")
    message(FATAL_ERROR "${MESSAGE_TEXT}")
endfunction()

list(LENGTH SERVER_FILE_STEMS SERVER_COUNT)
math(EXPR LAST_SERVER_INDEX "${SERVER_COUNT} - 1")
foreach(SERVER_INDEX RANGE ${LAST_SERVER_INDEX})
    list(GET SERVER_FILE_STEMS ${SERVER_INDEX} SERVER_FILE_STEM)
    list(GET SERVER_COMMON_NAMES ${SERVER_INDEX} SERVER_COMMON_NAME)

    set(STAGING_KEY "${STAGING_DIR}/${SERVER_FILE_STEM}.key")
    set(STAGING_CERT "${STAGING_DIR}/${SERVER_FILE_STEM}.crt")
    set(STAGING_PIN "${STAGING_DIR}/${SERVER_FILE_STEM}.pin")

    execute_process(
        COMMAND "${OPENSSL_COMMAND}" req
            -x509
            -newkey ec
            -pkeyopt ec_paramgen_curve:prime256v1
            -sha256
            -days 3650
            -noenc
            -batch
            -subj "/CN=${SERVER_COMMON_NAME}"
            -addext "basicConstraints=critical,CA:FALSE"
            -addext "keyUsage=critical,digitalSignature"
            -addext "extendedKeyUsage=serverAuth,clientAuth"
            -keyout "${STAGING_KEY}"
            -out "${STAGING_CERT}"
        RESULT_VARIABLE GENERATE_RESULT
        OUTPUT_VARIABLE GENERATE_OUTPUT
        ERROR_VARIABLE GENERATE_ERROR)
    if (NOT GENERATE_RESULT EQUAL 0)
        fail_generation(
            "Failed to generate the ${SERVER_FILE_STEM} certificate:\n"
            "${GENERATE_OUTPUT}${GENERATE_ERROR}")
    endif()

    execute_process(
        COMMAND "${OPENSSL_COMMAND}" pkey
            -in "${STAGING_KEY}" -check -noout
        RESULT_VARIABLE KEY_CHECK_RESULT
        OUTPUT_VARIABLE KEY_CHECK_OUTPUT
        ERROR_VARIABLE KEY_CHECK_ERROR)
    if (NOT KEY_CHECK_RESULT EQUAL 0)
        fail_generation(
            "The generated ${SERVER_FILE_STEM} private key failed "
            "validation:\n${KEY_CHECK_OUTPUT}${KEY_CHECK_ERROR}")
    endif()

    execute_process(
        COMMAND "${OPENSSL_COMMAND}" verify
            -CAfile "${STAGING_CERT}"
            -purpose sslserver
            "${STAGING_CERT}"
        RESULT_VARIABLE SERVER_CERT_CHECK_RESULT
        OUTPUT_VARIABLE SERVER_CERT_CHECK_OUTPUT
        ERROR_VARIABLE SERVER_CERT_CHECK_ERROR)
    if (NOT SERVER_CERT_CHECK_RESULT EQUAL 0)
        fail_generation(
            "The generated ${SERVER_FILE_STEM} certificate failed server "
            "validation:\n"
            "${SERVER_CERT_CHECK_OUTPUT}${SERVER_CERT_CHECK_ERROR}")
    endif()

    execute_process(
        COMMAND "${OPENSSL_COMMAND}" verify
            -CAfile "${STAGING_CERT}"
            -purpose sslclient
            "${STAGING_CERT}"
        RESULT_VARIABLE CLIENT_CERT_CHECK_RESULT
        OUTPUT_VARIABLE CLIENT_CERT_CHECK_OUTPUT
        ERROR_VARIABLE CLIENT_CERT_CHECK_ERROR)
    if (NOT CLIENT_CERT_CHECK_RESULT EQUAL 0)
        fail_generation(
            "The generated ${SERVER_FILE_STEM} certificate failed client "
            "validation:\n"
            "${CLIENT_CERT_CHECK_OUTPUT}${CLIENT_CERT_CHECK_ERROR}")
    endif()

    # Produce the conventional SPKI pin:
    # base64(SHA-256(DER SubjectPublicKeyInfo)).
    execute_process(
        COMMAND "${OPENSSL_COMMAND}" x509
            -in "${STAGING_CERT}" -pubkey -noout
        COMMAND "${OPENSSL_COMMAND}" pkey -pubin -outform DER
        COMMAND "${OPENSSL_COMMAND}" dgst -sha256 -binary
        COMMAND "${OPENSSL_COMMAND}" base64 -A
        RESULTS_VARIABLE PIN_RESULTS
        OUTPUT_VARIABLE PIN_TEXT
        ERROR_VARIABLE PIN_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    foreach(PIN_RESULT IN LISTS PIN_RESULTS)
        if (NOT "${PIN_RESULT}" STREQUAL "0")
            fail_generation(
                "Failed to generate the ${SERVER_FILE_STEM} SPKI pin:\n"
                "${PIN_ERROR}")
        endif()
    endforeach()

    string(LENGTH "${PIN_TEXT}" PIN_LENGTH)
    if (NOT PIN_LENGTH EQUAL 44
        OR NOT PIN_TEXT MATCHES "^[A-Za-z0-9+/]+=$")
        fail_generation(
            "OpenSSL produced an invalid SHA-256 SPKI pin for "
            "${SERVER_FILE_STEM}.")
    endif()
    file(WRITE "${STAGING_PIN}" "${PIN_TEXT}\n")

    # Confirm that the private key and certificate have the same SPKI.
    execute_process(
        COMMAND "${OPENSSL_COMMAND}" pkey
            -in "${STAGING_KEY}" -pubout
        COMMAND "${OPENSSL_COMMAND}" pkey -pubin -outform DER
        COMMAND "${OPENSSL_COMMAND}" dgst -sha256 -binary
        COMMAND "${OPENSSL_COMMAND}" base64 -A
        RESULTS_VARIABLE KEY_PIN_RESULTS
        OUTPUT_VARIABLE KEY_PIN_TEXT
        ERROR_VARIABLE KEY_PIN_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    foreach(KEY_PIN_RESULT IN LISTS KEY_PIN_RESULTS)
        if (NOT "${KEY_PIN_RESULT}" STREQUAL "0")
            fail_generation(
                "Failed to validate the ${SERVER_FILE_STEM} private key "
                "SPKI:\n${KEY_PIN_ERROR}")
        endif()
    endforeach()
    if (NOT KEY_PIN_TEXT STREQUAL PIN_TEXT)
        fail_generation(
            "The generated ${SERVER_FILE_STEM} certificate and private key "
            "do not match.")
    endif()
endforeach()

# Publish only after every generated credential has passed validation.
foreach(SERVER_FILE_STEM IN LISTS SERVER_FILE_STEMS)
    foreach(EXTENSION IN ITEMS key crt pin)
        file(COPY_FILE
            "${STAGING_DIR}/${SERVER_FILE_STEM}.${EXTENSION}"
            "${REMOTE_CREDENTIALS_DIR}/${SERVER_FILE_STEM}.${EXTENSION}")
    endforeach()
    file(CHMOD "${REMOTE_CREDENTIALS_DIR}/${SERVER_FILE_STEM}.key"
        PERMISSIONS OWNER_READ OWNER_WRITE)
endforeach()
file(REMOVE_RECURSE "${STAGING_DIR}")

message(STATUS
    "Generated Account, Chat, and World server credentials in:\n"
    "    ${REMOTE_CREDENTIALS_DIR}")
message(STATUS "")
message(STATUS "Next steps:")
message(STATUS
    "  1. Back up the .key and .crt files in secure storage.")
message(STATUS
    "  2. Commit the .pin files to the project repository.")
message(STATUS
    "  3. Manually deploy each server's .key and .crt files next to its "
    "executable.")
