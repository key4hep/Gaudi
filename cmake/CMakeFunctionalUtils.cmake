# .rst:
# CMakeFunctionalUtils
# --------------------
#
# This module provides some utility functions to simplify working with lists.
#
# - list_pop_front(<list> <output variable1> [<output variable2>...])
#
#     remove the first N entries of <list> and store them in the N variables
#     provided
#
# - list_zip(<output list> [<list1> <list2> ...])
#
#     generate a list interleaving the elements of the provided lists such that
#     we get first all the first elements then all the second elements, etc. up
#     to the last element of the shortest input list
#
# - list_unzip(<input list> <output list1> [<output list2> ...])
#
#     the inverse of list_zip
#
macro(list_pop_front input_list output)
  list(GET ${input_list} 0 ${output})
  list(REMOVE_AT ${input_list} 0)
  if(NOT "" STREQUAL "${ARGN}") # not better way to check if ARGN is empty
    list_pop_front(${input_list} ${ARGN})
  endif()
endmacro()

function(list_zip output)
  set(output_list)
  set(chunk TRUE) # dummy value to enter the loop
  while(chunk)
    set(chunk)
    foreach(input ${ARGN})
      if(${input})
        list_pop_front(${input} v)
        set(chunk ${chunk} ${v})
      else()
        set(chunk)
        break()
      endif()
    endforeach()
    set(output_list ${output_list} ${chunk})
  endwhile()
  set(${output} ${output_list} PARENT_SCOPE)
endfunction()

function(list_unzip input out1)
  # note: the argument 'out1' guarantees that we have at least one output list
  while(${input})
    foreach(output ${out1} ${ARGN})
      list_pop_front(${input} v)
      set(${output}_new ${${output}_new} ${v})
    endforeach()
  endwhile()
  foreach(output ${out1} ${ARGN})
    set(${output} ${${output}_new} PARENT_SCOPE)
  endforeach()
endfunction()
