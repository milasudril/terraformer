# Plateau measurements

Elevation category| Object                      | Elevation/m
------------------|-----------------------------|------------
Low               | Hardangervidda              |1100
High              | Tibetan Plateau, Mansarovar |4600

# Curve fitting rule

Use 6 elevation divisions, and model elevation as

$$
z = A\sinh(k n)
$$

Use n = 5 for Mansarovar, and n = 3 for Hardangervidda

Gives

Parameter | Value
----------|----------------
k         | 0.7086205026374324
A         | 266.3185546307779 m

# Results

Elevation category  | Flatland elevation/m
--------------------|---------------------
costal_lowland      | 0
continental_lowland | 200
midlands            | 520
highlands           | 1100
low_alpine_plain    | 2300
high_alpine_plain   | 4600