#.rst:
# CMakeDependentOption
# --------------------
#
# Macro to provide a cache veriable dependent on other options.
#
# This macro presents a cache variable to the user only if a set of other
# conditions are true.  When the variable is not presented a default value
# is used, but any value set by the user is preserved for when the
# variable is presented again.  Example invocation:
#
# ::
#
#   CMAKE_DEPENDENT_VARIABLE(FOO "Value of Foo" STRING "val"
#                          "USE_BAR;NOT USE_ZOT" "default_val")
#
# If USE_BAR is true and USE_ZOT is false, this provides a string cache variable
# called FOO that defaults to "val".  Otherwise, it sets FOO to
# "default_val".  If the status of USE_BAR or USE_ZOT ever changes, any value for
# the FOO variable is saved so that when the variable is re-enabled it
# retains its old value.

macro(CMAKE_DEPENDENT_VARIABLE var doc type default depends force)
  if(${var}_ISSET MATCHES "^${var}_ISSET$")
    set(${var}_AVAILABLE 1)
    foreach(d ${depends})
      string(REGEX REPLACE " +" ";" CMAKE_DEPENDENT_OPTION_DEP "${d}")
      if(${CMAKE_DEPENDENT_OPTION_DEP})
      else()
        set(${var}_AVAILABLE 0)
      endif()
    endforeach()
    if(${var}_AVAILABLE)
	  set(${var} ${default} CACHE ${type} ${doc})
	  set(${var} "${${var}}" CACHE ${type} ${doc} FORCE)
    else()
      if(${var} MATCHES "^${var}$")
      else()
        set(${var} "${${var}}" CACHE INTERNAL "${doc}")
      endif()
      set(${var} ${force})
    endif()
  else()
    set(${var} "${${var}_ISSET}")
  endif()
endmacro()
