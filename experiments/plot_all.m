function plot_all(path)
	hold on
	files = dir(fullfile(path, '*.txt'));
	l = length(files);
	for i = 1:l
		data = load(fullfile(path ,files(i).name));
		plot(data(:,1), data(:,2));
	end
	axis equal
end
