#include "py/builtin.h"
#include "py/obj.h"
#include <string.h>
#include "mphalport.h"

mp_obj_t mp_builtin_input(size_t n_args, const mp_obj_t *args) {
  // 1 - Retrieve the prompt if any
  const char * prompt = NULL;
  if (n_args == 1) {
    prompt = mp_obj_str_get_str(args[0]);
  }

  // 2 - Perform the HAL input command. This logs the prompt and the result
  const char * result = mp_hal_input(prompt);

  // 3 - Return the input
  mp_obj_t resultStr = mp_obj_new_str(result, strlen(result));
  return resultStr;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_builtin_input_obj, 0, 1, mp_builtin_input);
