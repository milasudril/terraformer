function rasterwave(imagefile, max_duration)
	data=imread(imagefile);
	x=[1:size(data,1)]';
	X=repmat(x, 1, size(data,2));
	colsum=sum(data);
	y=sum(data(:, [1:size(data,2)]').*X)./colsum;
	Y=abs(fft(y));
	fs=length(Y)/max_duration;
	f=linspace(0, fs, length(Y));
	plot(1.0./f(2:length(f)/2), Y(2:length(Y)/2), '.-');
end