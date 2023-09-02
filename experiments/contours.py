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
	u = inp.read_image(0, 0, 0, nchannels, 'float')
	inp.close()

inp = oiio.ImageInput.open(sys.argv[2])
if inp:
	spec = inp.spec()
	xres = spec.width
	yres = spec.height
	nchannels = spec.nchannels
	v = inp.read_image(0, 0, 0, nchannels, 'float')
	inp.close()


	matplotlib.pyplot.contour(numpy.flipud(u[:, :, 0]), levels = 16, extent = [0, 49152, 0, 49252], linewidths=0.75)
	matplotlib.pyplot.contour(numpy.flipud(v[:, :, 0]), levels = 16, extent = [0, 49152, 0, 49252], linewidths=0.75)
	matplotlib.pyplot.colorbar()
	ax = matplotlib.pyplot.gca()
	ax.set_aspect('equal', adjustable='box')
	matplotlib.pyplot.show()
