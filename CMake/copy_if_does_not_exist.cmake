# Copies a file only if it doesn't already exist at the destination.
function(copy_if_does_not_exist fileRelPath)
    set(inputFilePath "${PROJECT_SOURCE_DIR}/${fileRelPath}")
    set(outputDir "${CMAKE_CURRENT_BINARY_DIR}/")
    get_filename_component(outputFileName ${fileRelPath} NAME)
    set(outputFilePath "${outputDir}${outputFileName}")
    file(
        GENERATE 
            OUTPUT ${outputFilePath}
            CONTENT 
                "if (NOT EXISTS ${outputFilePath})
                    execute_process(
                         COMMAND \"${CMAKE_COMMAND}\" -E copy
                             ${inputFilePath}
                             ${outputDir}
                    )
                endif()"
    )
endfunction()
