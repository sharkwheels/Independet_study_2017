########################
# Flask-restful api example
# its todo lists! But its just a test
########################

from flask import Flask
from flask_restful import reqparse, abort, Api, Resource

app = Flask(__name__)
api = Api(app)

TODOS = {
	'todo1': {'task': 'build an API'},
	'todo2': {'task': 'what?'},
	'todo3': {'task': 'cloudy'}
}

def abort_if_todo_doesnt_exist(todo_id):
	if todo_id not in TODOS:
		abort(404,message="Todo{} doesn't exist".format(todo_id))

parser = reqparse.RequestParser()
parser.add_argument('task')

# Todo
# shows a single todo item and lets you delete a todo item

class HelloWorld(Resource):
	def get(self):
		return {'hello': 'world'}

class Todo(Resource):
	def get(self,todo_id):
		abort_if_todo_doesnt_exist(todo_id)
		return TODOS[todo_id]

	def delete(self,todo_id):
		abort_if_todo_doesnt_exist(todo_id)
		return TODOS[todo_id]

	def put(self,todo_id):
		args = parser.parse_args()
		task = {'task':args['task']}
		TODOS[todo_id]
		return '',204

# TodoList
# shows a list of all todos, and lets you POST to add new tasks

class TodoList(Resource):
	def get(self):
		return TODOS
	def post(self):
		args = parser.parse_args()
		todo_id = int(max(TODOS.keys()).lstrip('todo')) + 1
		todo_id = 'todo%i' % todo_id
		TODOS[todo_id] = {'task': args['task']}
		return TODOS[todo_id], 201
##
## Actually setup the Api resource routing here
##
api.add_resource(TodoList, '/todos')
api.add_resource(Todo, '/todos/<todo_id>')
api.add_resource(HelloWorld, '/')

if __name__ == '__main__':
	app.run(host='0.0.0.0')