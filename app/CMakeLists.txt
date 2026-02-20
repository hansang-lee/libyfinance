get_filename_component(DIRECTORY_PATH ${CMAKE_CURRENT_LIST_DIR} ABSOLUTE)
string(REPLACE "/" "_" DIRECTORY_NAME ${DIRECTORY_PATH})
macro(BUILD_APP "NAME")
  set("APP" ${DIRECTORY_NAME}_${NAME})
  add_executable(
    ${APP}
      ${NAME}.cpp
  )
  target_link_libraries(
    ${APP} PRIVATE
      yfinance::yfinance
  )
  set_target_properties(
    ${APP} PROPERTIES
      OUTPUT_NAME ${NAME}
      DEBUG_POSTFIX d
  )
endmacro()

BUILD_APP(fng)
BUILD_APP(stock)
