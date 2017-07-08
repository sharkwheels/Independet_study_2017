#########
# testing google maps api for directions
# routes you through a bunch of shops before sending you to your destination
########

# resources

# http://mkweb.bcgsc.ca/googlemapschallenge/
# https://github.com/googlemaps/google-maps-services-python
# https://developers.google.com/maps/documentation/directions/intro#Waypoints
# https://www.dataquest.io/blog/python-api-tutorial/
# http://py-googlemaps.sourceforge.net/
# https://googlemaps.github.io/google-maps-services-python/docs/genindex.html

import googlemaps
import os
from datetime import datetime
from googlemaps import convert
from googleplaces import GooglePlaces, types, lang

key = 'xxxxx'

## I realize it might be dumb to use TWO helpers, but eh. 
gmaps = googlemaps.Client(key=key)
gplaces = GooglePlaces(key)

## ppints
start_point = "start"
end_point = "finish"

way_points = []
# Request directions 
now = datetime.now()
directions = []




places_result = gplaces.nearby_search(location=start_point,
										keyword="pizza",
										radius=1000,
										types=[types.TYPE_FOOD])

## playing with places
## grabbing 5 places near the start address

if places_result.has_attributions:
	print(places_result.html_attributions)

limit = 4 

for index, place in enumerate(places_result.places):
	print(place.name)
	place.get_details()
	print(place.details['formatted_address'])
	way_points.append(place.details['formatted_address'])

	## break the loop if we hit the limit
	if index == limit:
		break

directions_result = gmaps.directions(start_point,
									 end_point,
									 mode="driving",
									 departure_time=now,
									 waypoints=convert.location_list(way_points))

## just playing w/ directions and waypoints
## setting those grabbed places as waypoints

for x in directions_result:
	print(x['warnings'])
	print(x['summary'])
	l = x['legs']
	for i in l:
		print(i['start_address'])
		print(i['end_address'])
		print(i['duration']['text'])
		print(i['distance']['text'])
		s = i['steps']
		for y in s:
			#print(y['html_instructions'])
			directions.append(y['html_instructions'])

## print(directions)
## could 

for i in directions:
	print(i)

"""
for place in places_result.places:
	# Returned places from a query are place summaries.
	print(place.name)
	print(place.geo_location)
	print(place.place_id)
	place.get_details()
	print(place.details)
	
"""
"""
legs = directions_result['legs']

for i in legs:
	print(i['duration']['text'])
	print(i['start_address'])
	print(i['end_address'])
	print(i['html_instructions'])

"""
"""
# Geocoding an address
geocode_result = gmaps.geocode('100 McCaul Street')

# Look up an address with reverse geocoding
reverse_geocode_result = gmaps.reverse_geocode((40.714224, -73.961452))
"""