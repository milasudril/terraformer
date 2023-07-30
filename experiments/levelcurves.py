import OpenImageIO as oiio
import matplotlib.pyplot
import numpy

inp = oiio.ImageInput.open('test.exr')
if inp:
	spec = inp.spec()
	xres = spec.width
	yres = spec.height
	nchannels = spec.nchannels
	pixels = inp.read_image(0, 0, 0, nchannels, 'float')
	print(pixels[:, :, 0].ndim)
	inp.close()
	levels = numpy.linspace(0, 8192 + 512, 18)
	matplotlib.pyplot.contour(numpy.flipud(pixels[:, :, 0]), levels, extent = [0, 49152, 0, 49252], cmap='tab10')
	matplotlib.pyplot.colorbar()
	matplotlib.pyplot.show()