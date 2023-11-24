project('ckafprod', 'c', subproject_dir: 'lib', default_options: [
    'buildtype=debug', 
	'c_std=gnu11', 
	'b_lto=false',
	'default_library=static',
])

add_global_arguments('-I../include', language: 'c')

ckafprod_deps = []
ckafprod_sources = [
	'src/main.c',
	'src/utils.c',
	'src/json_parsing.c',
	'src/sim/timeutils.c',
	'src/sim/simulation.c',
	'src/sim/house_worker.c',

	'src/modifiers.c',
]

conf = configuration_data()

if get_option('use_vm')
	curl_dep = dependency('libcurl', required: false)
	if not curl_dep.found()
		error('libcurl not found! Please install libcurl4-openssl-dev or the appropriate package for your system.')
	endif
	ckafprod_deps += curl_dep

	conf.set('USE_VM', 1)
	ckafprod_sources += [
		'src/victoria_metrics.c',
	]
endif

ckafprod_sources += vcs_tag(input: files('src/version.c.in'), output: 'version.c', command: ['git', 'describe', '--always', '--dirty=+'])

ckafprod_deps += dependency('cjson', fallback : ['cjson', 'cjson_dep'], required: true)

ckafprod_config_h = configure_file(output: 'ckafprod_config.h', configuration: conf)
# install_subdir('include', install_dir : '.')
# install_headers(ckafprod_config_h, install_dir : 'include/ckafprod')

ckafprod = executable('ckafprod', [ckafprod_sources, ckafprod_config_h],
	dependencies : ckafprod_deps,
	link_args : ['-Wl,-Map=ckafprod.map', '-lm', '-Wl,--export-dynamic', '-ldl'],  # -ldl is needed on ARM/raspbarian
	install : true,
)

# Add .json file for QoL while testing
configure_file(input: 'houses.json', output: 'houses.json', copy: true)
