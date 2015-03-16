{
	"targets": [
		{
			"target_name": "q24",

			"sources": [
				"q24.cpp",
				"node-q24.cc"
			],

			"include_dirs" : [
				"<!(node -e \"require('nan')\")"
			]
		}
	]
}