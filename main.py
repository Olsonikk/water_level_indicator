#importing and extracting essential data from json
#this particular json comes from 'Warsaw-Bulwary' sensor

import requests, json, numpy
import matplotlib.pyplot as plt

temp=[]
dates=[]
values=[]

current_raw = requests.get("https://hydro-back.imgw.pl/station/hydro/status?id=152210170")
history_raw = requests.get("https://hydro-back.imgw.pl/station/hydro/status/data?id=152210170&hoursInterval=24")

current_json = current_raw.json()
history_json = history_raw.json()

location=current_json['status']['description']


#print(current_json['status'])
#print(history_json['operational'])


for row in history_json['operational']:
    if row['date'][14] != '0': #first 10 measurements are every 10 mins. We want results to be in 1 hour interval
        continue
    temp.append(row)

first_measurement=temp[0]['date']
last_measurement=temp[-1]['date']

for row in temp:
    dates.append(row['date'].split('T')[1][:5])
    values.append(row['value'])

#print(dates)
#print(values)

plt.figure(figsize=(16, 8))
plt.plot(dates,values)
plt.title(f"{location} from {first_measurement} to {last_measurement}")

plt.show()
