#include <py/obj.h>

mp_obj_t modpyplot___init__();
void modpyplot_gc_collect();

mp_obj_t modpyplot_arrow(size_t n_args, const mp_obj_t *args);
mp_obj_t modpyplot_axis(size_t n_args, const mp_obj_t *args);
mp_obj_t modpyplot_bar(mp_obj_t x, mp_obj_t height);
mp_obj_t modpyplot_grid(mp_obj_t b);
mp_obj_t modpyplot_hist(mp_obj_t x);
mp_obj_t modpyplot_plot(mp_obj_t x, mp_obj_t y);
mp_obj_t modpyplot_scatter(mp_obj_t x, mp_obj_t y);
mp_obj_t modpyplot_text(mp_obj_t x, mp_obj_t y, mp_obj_t s);
mp_obj_t modpyplot_show();