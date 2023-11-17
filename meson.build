project('ckafprod', 'c', subproject_dir: 'lib', default_options: [
    'buildtype=debug', 
	'c_std=gnu11', 
	'b_lto=false',
	'default_library=static',
])

add_global_arguments('-I../include', language: 'c')

ckafprod_sources = [
	'src/main.c',
	'src/utils.c',
	'src/json_parsing.c',
	'src/sim/simulation.c',
	'src/sim/house_worker.c',
	# 'src/sim/subscriptions.c',
]

ckafprod_sources += vcs_tag(input: files('src/version.c.in'), output: 'version.c', command: ['git', 'describe', '--always', '--dirty=+'])

cjson_dep = dependency('cjson', fallback : ['cjson', 'cjson_dep'], required: true)

ckafprod = executable('ckafprod', ckafprod_sources,
	dependencies : [cjson_dep],
	link_args : ['-Wl,-Map=csh.map', '-lm', '-Wl,--export-dynamic', '-ldl'],  # -ldl is needed on ARM/raspbarian
	install : true,
)

# Add .json file for QoL while testing
configure_file(input: 'houses.json', output: 'houses.json', copy: true)
