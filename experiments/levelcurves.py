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
	pixels = numpy.flipud(pixels[:, :, 0])
	inp.close()
	t = numpy.linspace(0, 16, 17)/16
	exp = t*13 + (1 - t)*9
	levels = numpy.power(2, exp)

	y, x = 48*numpy.mgrid[0:yres:16, 0:xres:16]
	dy, dx = numpy.gradient(pixels[::16, ::16])

	matplotlib.pyplot.contour(pixels, levels, extent = [0, 49152, 0, 49252], linewidths=0.75)
	matplotlib.pyplot.colorbar()
	matplotlib.pyplot.streamplot(x, y, -dx, -dy, linewidth=0.75)
	ax = matplotlib.pyplot.gca()
	ax.set_aspect('equal', adjustable='box')
	matplotlib.pyplot.show()
