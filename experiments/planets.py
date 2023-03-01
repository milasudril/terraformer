import json
import matplotlib
import math

with open('data/planets.json') as f:
    data = json.load(f)

d_0 = data['Earth']['distance_to_sun']
f_0 = 1/data['Earth']['period_of_revolution']

planets_filtered = []

for planet, info in data.items():
    if planet != 'Earth':
        dr = d_0 - info['distance_to_sun']
        m = info['mass']
        new_item = info
        new_item['strength'] = m/(dr*dr)
        new_item['name'] = planet
        planets_filtered.append(new_item)

planets_filtered = sorted(planets_filtered, key = lambda x:x['strength'], reverse = True)

print(planets_filtered)
T = 80000*365
N = 1024

dt = T/N
t = []
val = []
fa = 1/planets_filtered[0]['period_of_revolution']
ma = planets_filtered[0]['mass']
da = planets_filtered[0]['distance_to_sun']

fb = 1/planets_filtered[1]['period_of_revolution']
mb = planets_filtered[1]['mass']
db = planets_filtered[1]['distance_to_sun']
for k in range(N):
    t.append(k*dt);
    loc_0x = d_0*math.cos(2*math.pi*f_0*k*dt)
    loc_0y = d_0*math.sin(2*math.pi*f_0*k*dt)


    loc_ax = da*math.cos(2*math.pi*fa*k*dt);
    loc_ay = da*math.sin(2*math.pi*fa*k*dt);
    dx = loc_ax - loc_0x
    dy = loc_ay - loc_0y
    distance_squared = dx*dx + dy*dy
    Fax = dx*ma/math.pow(distance_squared, 3/2);


    loc_bx = db*math.cos(2*math.pi*fb*k*dt);
    loc_by = db*math.sin(2*math.pi*fb*k*dt);
    dx = loc_bx - loc_0x
    dy = loc_by - loc_0y
    distance_squared = dx*dx + dy*dy
    Fbx = dx*mb/math.pow(distance_squared, 3/2);

    val.append(Fax + Fbx);


import matplotlib.pyplot

matplotlib.pyplot.plot(t, val)
matplotlib.pyplot.show()