import OpenImageIO as oiio
import matplotlib.pyplot
import numpy
import sys

inp = oiio.ImageInput.open(sys.argv[1])
if inp:
	spec = inp.spec()
	xres = spec.width
	yres = spec.height
	nchannels = spec.nchannels
	pixels = inp.read_image(0, 0, 0, nchannels, 'float')
	print(pixels[:, :, 0].ndim)
	inp.close()
	t = numpy.linspace(0, 16, 17)/16
	exp = t*13 + (1 - t)*9
	levels = numpy.power(2, exp)
	matplotlib.pyplot.contour(numpy.flipud(pixels[:, :, 0]), levels, extent = [0, 49152, 0, 49252])
	matplotlib.pyplot.colorbar()
	matplotlib.pyplot.show()
