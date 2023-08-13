import OpenImageIO as oiio
import matplotlib.pyplot
import numpy

inp = oiio.ImageInput.open('data/annapurna_ridge.exr')
if inp:
	spec = inp.spec()
	xres = spec.width
	yres = spec.height
	nchannels = spec.nchannels
	pixels = inp.read_image(0, 0, 0, nchannels, 'float')
	inp.close()
	n = 8
	dx = xres/n
	matplotlib.pyplot.figure()
	x = range(0, yres)
	mean = numpy.zeros(yres)
	print(mean)
	for k in range(0, n):
		i = int((k + 0.5)*dx)
		mean = mean + pixels[:,i, 0]
		print(len(pixels[:,i, 0]))
	mean = mean / n

	for k in range(0, n):
		i = int((k + 0.5)*dx)
		matplotlib.pyplot.plot(x, pixels[:,i, 0] - mean)
	matplotlib.pyplot.show()



	#	print(i)


#	matplotlib.pyplot.contour(numpy.flipud(pixels[:, :, 0]), cmap='tab10')
#	matplotlib.pyplot.colorbar()
	#matplotlib.pyplot.show()
