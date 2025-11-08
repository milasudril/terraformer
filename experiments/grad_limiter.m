1;

function y = clamp(x, low, high)
	y = min(high, max(x, low));
end

function [z_out, max_grad] = step(z_in)
	z_out = zeros(size(z_in));
  max_grad = 0;
	for k = 1:length(z_in)
		left = max(k - 1, 1);
		right = min(k + 1, length(z_in));
		ddx = 0.5*(z_in(right) - z_in(left));
		if abs(ddx) > 1 || max(z_in(left), z_in(right)) - z_in(k) > 1
      uphill_index = 0;
      if abs(ddx) == 0
        if z_in(left) > z_in(right)
          uphill_index = left;
        else
          uphill_index = right;
        end
      else
        uphill_index = clamp(k + ddx/abs(ddx), 1, length(z_in));
      end
      required_value = z_in(uphill_index) - 1;
      current_value = z_in(k);
      error = min(current_value - required_value, 0);
      z_out(k) = z_in(k) - error;
		else
			z_out(k) = z_in(k);
		end
    max_grad = max(max_grad, abs(0.5*(z_in(right) - z_in(left))));
	end
end

x = linspace(0, 128, 257)/128;
z = 0.125*(cos(2.0*pi*x)
            + 0.5*sin(4.0*pi*x)
            - 0.25*cos(8.0*pi*x)
            - 0.125*sin(16.0*pi*x)
            + 0.0625*cos(32.0*pi*x)
            + 0.03125*sin(64.0*pi*x)
            - 0.015625*cos(128.0*pi*x));
plot(x, z)
axis equal
pause

for k=1:100
  [z, max_grad]=step(256*z);
  z = z/256;
  max_grad
  plot(x, z);
  axis equal;
  pause
end
