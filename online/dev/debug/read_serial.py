import subprocess

if __name__ == "__main__":

	p=subprocess.Popen(["ls", "-ltrh", "/dev/serial/by-id/"], stdout=subprocess.PIPE)
	out, err = p.communicate()
	out = out.decode("utf-8")
	
	lines = out.split("\n")
	CAEN_found = False
	MANGOlino_found = False
	KEG_found = False
	
	results = {}
	for i, l in enumerate(lines):
		if "tty" in l:
			if "CAEN" in l:
				print("CAEN: ", l, " - ", l[-1])
				CAEN_found = True
				results["HV"] = l[-1]
			elif "95736323532351C081D1" in l:
				print("MANGOlino: ", l, " - ", l[-1])
				MANGOlino_found = True
				results["MANGOlino"] = l[-1]
			elif "4423831383835190E090" in l:
				print("KEG: ", l, " - ", l[-1])
				KEG_found = True
				results["KEG"] = l[-1]
				
				
	if not CAEN_found:
		raise Exception("CAEN HV not found.")
	if not MANGOlino_found:
		raise Exception("MANGOlino Arduino not found.")
	if not KEG_found:
		raise Exception("KEG Arduino not found.")
		
		
	print("RESULTS:")
	print(results)
	
