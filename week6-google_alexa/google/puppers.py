import logging
from flask import Flask
from flask_assistant import Assistant, ask, tell, context_manager
import random
from random import choice

app = Flask(__name__)
assist = Assistant(app)
logging.getLogger('flask_assistant').setLevel(logging.DEBUG)

def getAction():
	list_to_say = [
		"I'm a good boy!",
		"Weeeeee! Pupper!",
		"Bork bork, you are doing me a concern friend",
		"Heckin' gonna be a pupper."
	]
	return random.choice(list_to_say)

@assist.action('greeting')
def greet_and_start():
	speech = "Hey! I'm a pupper, how are you feeling today?"
	return ask(speech)

@assist.action('user-gives-feeling')
def ask_for_play(feeling):
	cont = getAction()
	if feeling == "sad":
		msg = "Awww"
	elif feeling == "happy":
		msg = "bark bark!"
	elif feeling == "angry":
		msg = "Grrrrrr!"

	speech = msg + " {}".format(cont)
	return ask(speech)


if __name__ == '__main__':
	app.run(debug=True)
