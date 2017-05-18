import sys, getopt
import json
import requests
from pprint import pprint

def handleRequest(argv):
	"""Function to read user request, form http message and send it"""
	try:
		opts, args = getopt.getopt(argv,"",["help","version=","repo=","user=","apitoken=","updateversion="])
	except getopt.GetoptError:
		print 'usage: python deploy_pxscene.py --version=<version>  --repo=<repo> --user=<user> --apitoken=<apitoken> --updateversion=<true/false>'
		exit (1)
	
	#initialize input values
	version="";
	repo_name="pxCore";
	user_name="";
	api_token="";
	update_version="false";
	

	#read input values
	for opt, arg in opts:
		if opt == "--help":
			print 'python deploy_pxscene.py --version=<version>  --repo=<repo> --user=<user> --apitoken=<apitoken> --updateversion=<true/false>'
			exit (1)
		elif len(arg) == 0:
			print "argument cannot be empty for option", opt
			exit (1)
		elif opt == "--version":
			version=arg;
		elif opt == "--repo":
			repo_name=arg;
		elif opt == "--user":
			user_name=arg;
		elif opt == "--apitoken":
			api_token=arg;
		elif opt == "--updateversion":
			update_version=arg;

	if version=="" or user_name=="" or api_token=="":
		print 'usage: python deploy_pxscene.py --version=<version>  --repo=<repo> --user=<user> --apitoken=<apitoken> --updateversion=<true/false>'
		exit (1)

	if update_version != "true" and update_version != "false":
		print ("please enter proper update version value true/false");
		exit (1)
	
	with open('pxscene_deploy_rules.json') as data_file:
	    data = json.load(data_file)
	
	#populate environment variables
	data["request"]["config"][unicode("env")] = {}
	data["request"]["config"]["env"][unicode("PX_VERSION")] = unicode(version);
	data["request"]["config"]["env"][unicode("REPO_USER_NAME")] = unicode(user_name);
	data["request"]["config"]["env"][unicode("REPO_NAME")] = unicode(repo_name);
	data["request"]["config"]["env"][unicode("UPDATE_VERSION")] = unicode(update_version);
	string = json.dumps(data);
	
	#print the request json data
	pprint(string);
	
	tokendata = "token ";
	tokendata += str(api_token);
	
	#populate http header
	headers = {}
	headers["Content-Type" ] = "application/json";
	headers["Accept" ] = "application/json";
	headers["Travis-API-Version"] = "3";
	headers["Authorization"] = tokendata;
	
	url = "https://api.travis-ci.org/repo/" + str(user_name) + "%2F" + str(repo_name) + "/requests";
	
	#send http request
	response = requests.post(url, headers=headers, data=string)
	print response.content

if __name__ == "__main__":
	if len(sys.argv) < 2:
		print 'usage: python deploy_pxscene.py --version=<version>  --repo=<repo> --user=<user> --apitoken=<apitoken> --updateversion=<true/false>'
		exit (1)
	handleRequest(sys.argv[1:])
