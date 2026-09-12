function(am_check_tls_pins_exist)
    if (ARGC EQUAL 0)
        message(FATAL_ERROR
            "am_check_tls_pins_exist requires at least one pin file.")
    endif()

    foreach(REMOTE_TLS_PIN IN LISTS ARGN)
        string(CONCAT REMOTE_TLS_PIN_CHECK
            "if (NOT EXISTS \"${REMOTE_TLS_PIN}\")\n"
            "    message(FATAL_ERROR \"Required remote TLS pin is missing:"
            "\\n\\n    ${REMOTE_TLS_PIN}\\n\\nRemote TLS pins identify the project's "
            "deployed servers and are normally committed to the project "
            "repository. See the \\\"TLS Credentials\\\" section of the "
            "Amalgam Engine README.\")\n"
            "endif()")
        install(CODE "${REMOTE_TLS_PIN_CHECK}")
    endforeach()
endfunction()
