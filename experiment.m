clear all

y_k = 0;

y=zeros(1024,1);

alpha = 1/32

for k = 1:length(y)
	x_k=2*(rand() - 0.5);
	y_k = alpha*x_k + (1 - alpha)*y_k;
	y(k) = y_k;
end

plot(exp(4*y), '-', 'linewidth', 1)