
type t_gb_testing
  field bitmap :t_bitmap
  field image  :t_image
  field color  :t_color
endtype

global gb_testing :t_gb_testing

function gb_testing_init()
	gb_testing = new t_gb_testing
  gb_testing.image = new_image("gb/media/images/test.png")
  gb_testing.bitmap = new_bitmap_from_image(gb_testing.image)
  gb_testing.color = new_color(0,0,0,3)
endfunction

function gb_testing_load()
endfunction

function gb_testing_start()
endfunction

function gb_testing_update(d:float)
endfunction

function gb_testing_draw(x:float=0, y:float=0)

  bitmap_draw_rect_c(gb_testing.bitmap, rand(0,99), rand(0,99), 10, 10, gb_testing.color)
  bitmap_sync(gb_testing.bitmap)

  bitmap_draw(gb_testing.bitmap, x+10, y)
  image_draw(gb_testing.image, x, y)
endfunction

function gb_testing_end()
endfunction