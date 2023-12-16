# Damped oscillation

$$
\ddot{y} + 2\zeta\omega_0\dot{y} + \omega_0^2 y = 0
$$


## Poles
The charactersitic polynomial for the equation is
$$
r^2 + 2\zeta\omega_0r + \omega_0^2 = \left(r + \zeta\omega_0\right)^2 - \zeta^2\omega_0^2 + \omega_0^2 = \left(r + \zeta\omega_0\right)^2 - \omega_0^2\left(\zeta^2 - 1\right)
$$
with zeros given by

$$
\left(r + \zeta\omega_0\right)^2 = \omega_0^2\left(\zeta^2 - 1\right)
$$

Assume $\zeta < 1$. Then
$$
r = -\zeta\omega_0 \pm \mathrm{i}\omega_0\sqrt{1 - \zeta^2}
$$

## Driven oscillation
Let $x$ be an input signal. Assuming it has the same unit as $y$, it is reasonable to use the equaition
$$
\ddot{y} + 2\zeta\omega_0\dot{y} + \omega_0^2 y = \omega_0^2 x
$$

## Cutoff frequency from eigen-frequency
$$
\omega_e = \omega_0\sqrt{1 - \zeta^2} \Leftrightarrow \omega_0 = \frac{\omega_e}{\sqrt{1 - \zeta^2}}
$$

## Discretization
Let $\dot{y} = v$

Then
$$
\left\{
\begin{aligned}
\dot{v} + 2\zeta\omega_0v + \omega_0^2 y &= \omega_0^2 x\\
 \dot{y}&=v
\end{aligned}
\right.\Leftrightarrow
\left\{
\begin{aligned}
\dot{v} &= \omega_0^2 x  -  2\zeta\omega_0v - \omega_0^2 y\\
 \dot{y}&=v
\end{aligned}
\right.
$$

Using the trapetzoidal rule
$$
\left\{
\begin{aligned}
v_n &= v_{n - 1} + \frac{h}{2}\left(\dot{v}_n + \dot{v}_{n - 1}\right)\\
y_n &= y_{n - 1} + \frac{h}{2}\left(\dot{y}_n + \dot{y}_{n - 1}\right)
\end{aligned}
\right.
$$ and
$$
y_n = y_{n - 1} + \frac{h}{2}\left(v_n + v_{n - 1}\right)
$$

This gives the system of equations
$$
\begin{aligned}
v_n &= v_{n - 1} + \frac{h}{2}\left(\dot{v}_n + \dot{v}_{n - 1}\right) \\
 &= v_{n - 1} + \frac{h}{2}\left(\omega_0^2 x_n  -  2\zeta\omega_0v_n - \omega_0^2 y_n + \omega_0^2 x_{n - 1}  -  2\zeta\omega_0v_{n - 1} - \omega_0^2 y_{n - 1}\right) \\
 &= v_{n - 1} + \frac{h}{2}\left(\omega_0^2\left(x_{n + 1} + x_n\right) - 2\zeta\omega_0\left(v_n + v_{n - 1} \right) - \omega_0^2\left(y_n +  y_{n - 1}\right)\right)
\end{aligned}
$$

