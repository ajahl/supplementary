if(EXISTS ${WORKING_DIRECTORY}/${OUTPUT})
  message(STATUS "${OUTPUT} exists. So do not generate it.")
else()
  message(STATUS "${OUTPUT} doesn't exist.")
  set(re2cInput ${WORKING_DIRECTORY}/${INPUT})
  set(re2cOutput ${WORKING_DIRECTORY}/${OUTPUT})
  execute_process(
    COMMAND re2c -o ${re2cOutput} ${re2cInput}
  )
endif()