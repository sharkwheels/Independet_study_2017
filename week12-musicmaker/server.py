#########################
# All Star Interrupt
# This is a form and a server
# It is meant to run locally
# do not upload this thing, its just a test for trying out form buttons. 
#############################

#https://blog.restsecured.xyz/writing-an-api-with-flask-restful-part-1-61b0e26e0e5b

import logging
import datetime
import json
import time
import random
import os
import urllib.request

from flask import Flask, render_template, request, redirect, url_for, flash
from flask_restful import Resource, Api
from threading import Thread
from time import sleep

logging.getLogger('flask_ask').setLevel(logging.DEBUG)

app = Flask(__name__, static_folder='static',static_url_path='/static')
app.config['DEBUG'] = True
app.config['TRAP_BAD_REQUEST_ERRORS'] = True

api = Api(app)

def resetFunction():
	global state
	state = "off"
	print('ran the reset')

def resetTimer(seconds):
	sleep(seconds)
	resetFunction()


@app.route('/',methods=['GET','POST'])
def main():
	return render_template('form.html') 

@app.route("/submit-form", methods = ['POST'])
def doIt():
	print("button pressed!")
	try:
		global state 
		state = "on"
		resetThread = Thread(target=resetTimer, args=(20,)) ## X seconds
		resetThread.start()
		return redirect(url_for('thankYou'))
	except ValueError: 
		return redirect(url_for('oops'))


@app.route('/about')
def about():
	return "An an about page."
	
@app.route("/thanks", methods=['GET'])
def thankYou():
	return render_template('thanks.html')


class AllStar(Resource):
	
	global state
	state = "off"
	
	def get(self):
		return {'state': state}

api.add_resource(AllStar, '/allstar')

if __name__ == '__main__':

	app.run(host="0.0.0.0", debug=True, use_reloader=False)