// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>
var Q = require('./build_utils/q.js');
var argv = require('./build_utils/minimist')(process.argv.slice(2));
var fs = require('fs');

var print = function(text) {
  text = ' ' + text;
  for(var i = 0; i < print.indent; ++i) {
    text = '-' + text;
  }
  console.log(text);
};
print.begin = function(text) {
  this(text);
  this.indent += 2;
};
print.end = function(text) {
  this.indent -= 2;
  if(text) this(text);
};
print.indent = 1;

function run(command, options) {
  var deferred = Q.defer();
  var args = command.split(' ');
  command = args[0];
  args = args.splice(1);
  options = options || {};
  options['stdio'] = 'inherit';
  var proc = require('child_process').spawn(command, args, options);
  proc.on('close', function(code) {
    if (code) {
      deferred.reject(command + ": " + code);
    } else {
      deferred.resolve();
    }
  });
  return deferred.promise;
}

function git(url, parent, name) {
  var opts = { cwd: parent };
  if (fs.existsSync(parent + name)) {
    print('Updating '+name+'...');
    return run('git pull', opts);
  } else {
    print('Cloning '+name+'...');
    return run('git clone ' + url + ' ' + name, opts);
  }
}

var debug = argv.debug || false;
var compiler = 'clang++';
var flags = ['-std=c++11', '-O0', '-g'];
var libs = [];

function download_depot_tools(opts) {
  return git('https://chromium.googlesource.com/chromium/tools/depot_tools.git',
             'third_party/', 'depot_tools');
}

function mkdir(path) {
  if(!fs.existsSync(path))
    fs.mkdirSync(path);
}

function extend(destination, source) {   
  for (var property in source) {
    destination[property] = source[property];
  }
  return destination;
};

function prepare_skia() {
  print.begin("Preparing skia");
  flags = flags.concat([
    "-DSK_INTERNAL",
    "-DSK_GAMMA_SRGB",
    "-DSK_GAMMA_APPLY_TO_A8",
    "-DSK_SCALAR_TO_FLOAT_EXCLUDED",
    "-DSK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1",
    "-DSK_SUPPORT_GPU=1",
    "-DSK_SUPPORT_OPENCL=0",
    "-DSK_FORCE_DISTANCEFIELD_FONTS=0",
    "-DSK_SCALAR_IS_FLOAT",
    "-DSK_SAMPLES_FOR_X",
    "-DSK_BUILD_FOR_UNIX",
    "-DSK_USE_POSIX_THREADS",
    "-DSK_SUPPORT_PDF",
    "-DSK_SYSTEM_ZLIB=1"
    //"-DSK_RELEASE",
    //"-DNDEBUG"
  ]);
  flags = flags.concat([
    "-m64",
    "-msse2",
    "-mfpmath=sse",
    "-g",
    "-fno-exceptions",
    "-fstrict-aliasing",
    "-Wall",
    "-Wextra",
    "-Winit-self",
    "-Wpointer-arith",
    "-Wno-c++11-extensions",
    "-Wno-unused-parameter",
    "-Wno-unused-variable",
    "-Werror",
    //"-O3",
    "-fno-rtti",
    "-Wnon-virtual-dtor",
    "-Wno-invalid-offsetof"
  ]);
  flags = flags.concat([
    "animator", "pdf", "config", "core", "pathops", "pipe", "effects", "images",
    "ports", "utils", "gpu", "views", "views/animated", "xml" ].map(
      function(name) { return "-Ithird_party/skia/include/" + name;}));
  libs = libs.concat([
    '-Wl,--start-group',
    "third_party/skia/out/Debug/libskia_animator.a",
    "third_party/skia/out/Debug/obj/gyp/libetc1.a",
    "third_party/skia/out/Debug/obj/gyp/libexperimental.a",
    "third_party/skia/out/Debug/obj/gyp/libflags.a",
    "third_party/skia/out/Debug/obj/gyp/liblua.a",
    "third_party/skia/out/Debug/libskia_pdf.a",
    "third_party/skia/out/Debug/obj/gyp/libresources.a",
    "third_party/skia/out/Debug/obj/gyp/libsk_tool_utils.a",
    "third_party/skia/out/Debug/libskia_views.a",
    "third_party/skia/out/Debug/obj/gyp/libviews_animated.a",
    "third_party/skia/out/Debug/libskia_xml.a",
    "third_party/skia/out/Debug/libskia_skgputest.a",
    "third_party/skia/out/Debug/libskia_core.a",
    "third_party/skia/out/Debug/libskia_effects.a",
    "third_party/skia/out/Debug/libskia_images.a",
    "third_party/skia/out/Debug/obj/gyp/libjpeg.a",
    "third_party/skia/out/Debug/obj/gyp/libSkKTX.a",
    "third_party/skia/out/Debug/obj/gyp/libwebp_dec.a",
    "third_party/skia/out/Debug/obj/gyp/libwebp_demux.a",
    "third_party/skia/out/Debug/obj/gyp/libwebp_dsp.a",
    "third_party/skia/out/Debug/obj/gyp/libwebp_enc.a",
    "third_party/skia/out/Debug/obj/gyp/libwebp_utils.a",
    "third_party/skia/out/Debug/libskia_utils.a",
    "third_party/skia/out/Debug/obj/gyp/libgiflib.a",
    "third_party/skia/out/Debug/libskia_opts.a",
    "third_party/skia/out/Debug/libskia_opts_ssse3.a",
    "third_party/skia/out/Debug/libskia_opts_sse4.a",
    "third_party/skia/out/Debug/libskia_ports.a",
    "third_party/skia/out/Debug/libskia_sfnt.a",
    "third_party/skia/out/Debug/libskia_skgpu.a",
    "third_party/skia/out/Debug/obj/gyp/libskflate.a",
    "-Wl,--end-group",
    '-lpng', '-lz', '-lpthread', '-lfontconfig', '-ldl', '-lfreetype',
    '-lGL', '-lGLU', '-lX11', '-lpng' ]);
  var env = extend({
    PATH: __dirname + '/third_party/depot_tools/:' + process.env.PATH,
    GYP_DEFINES: "skia_giflib_static=1",
    GYP_GENERATORS: "ninja"
  }, process.env);
  var path = __dirname + '/third_party/skia/';
  var opts = { cwd: path, env: env };
  var promise = Q();
  if(!argv['skip-deps']) {
    promise = promise.then(function() { 
      download_depot_tools(opts);
    }).then(function() {
      return mkdir(path);
    }).then(function() {
      return run("gclient config --name . --unmanaged https://skia.googlesource.com/skia.git", opts);
    }).then(function() {
      return run("gclient sync --deps barelinux", opts);
    }).then(function() {
      if(argv['skip-git']) return;
      return run("git checkout master", opts);
    }).then(function() {
      print("Compiling skia");
      return run("make", opts);
    });/*.then(function() {
      print("Packaging library");
      var libs = [
        "libskia_core.a",
        "libskia_ports.a",
        "libskia_opts.a",
        "libskia_utils.a",
        "libskia_skgpu.a",
        "libskia_opts_ssse3.a",
        "libskia_opts_sse4.a",
        "obj/gyp/libetc1.a",
        "obj/gyp/libSkKTX.a",
        "libskia_images.a",
        "libskia_effects.a",
        "libskia_sfnt.a",
        "libskia_views.a",
        "libskia_xml.a"
      ];
      return run("../../tools/merge_static_libs.py libskia.a " + libs.join(' '), { 
        cwd: opts.cwd + 'out/Debug'
      });
    });*/
  }
  return promise.then(function() {
    print.end("Skia ready");
  });
}

function prepare_glm() {
  print.begin("Preparing glm");
  flags.push("-Ithird_party/glm");
  flags.push("-D_REENTRANT");
  var promise = Q();
  if(!argv['skip-deps']) {  
    promise = promise.then(function() {
      return git('https://github.com/g-truc/glm.git', 'third_party/', 'glm');
    });
  }
  return promise.then(function() {
    print.end("glm ready");
  });
}

function prepare_nanosvg() {
  print.begin("Preparing nanosvg");
  flags.push("-Ithird_party/nanosvg/src");
  var promise = Q();
  if(!argv['skip-deps']) {  
    promise = promise.then(function() {
      return git('https://github.com/memononen/nanosvg.git', 'third_party/', 'nanosvg');
    });
  }
  return promise.then(function() {
    print.end("nanosvg ready");
  });
}

function prepare_SDL() {
  print.begin("Preparing SDL");
  flags.push('-Ithird_party/SDL/include');
  libs.push('third_party/SDL/build/libSDL2.a');
  libs.push('-lGL');
  libs.push('-ldl');
  libs.push('-lm');
  libs.push('-pthread');
  libs.push('-Wl,--no-undefined');
  var promise = Q();
  if(!argv['skip-deps']) {  
    var opts = {
      cwd: __dirname + '/third_party/SDL/', 
      env: extend({}, process.env)
    };
    promise =  promise.then(function() {
      return git("https://github.com/spurious/SDL-mirror.git", "./third_party/", "SDL");
    }).then(function() {
      return mkdir(__dirname + '/third_party/SDL/build/');
    }).then(function() {
      return run('cmake .. -DVIDEO_WAYLAND=0', { cwd: __dirname + '/third_party/SDL/build/' });
    }).then(function() {
      return run('make', { cwd: __dirname + '/third_party/SDL/build/' });
    });
  }
  return promise.then(function() {
    print.end("SDL ready");
  });
}

function prepare_dependencies() {
  print.begin("Preparing dependencies");
  return Q()
    .then(function() {
      mkdir(__dirname + '/third_party/');
    }).then(prepare_skia)
    .then(prepare_SDL)
    .then(prepare_glm)
    .then(prepare_nanosvg)
    .then(function() {
      print.end("Dependencies ready!");
    });
}

function build_constructor() {
  print.begin("Building constructor");
  var bundle = 'src/_bundle.cc';
  fs.writeFileSync(
    bundle, 
    fs.readdirSync(__dirname + '/src/')
      .filter(function(filename) { return /^.*\.cc$/.test(filename); })
      .filter(function(filename) { return !/^\.#.*$/.test(filename); })
      .filter(function(filename) { return !/^flycheck_.*$/.test(filename); })
      .map(function(filename) { return '#include "' + filename + '"'; })
      .join('\n') + '\n');
  var command = compiler +
    ' ' + flags.join(' ') +
    ' ' + bundle +
    ' ' + libs.join(' ') +
    ' -o build/final/constructor';
  return Q.fcall(function() {
    return mkdir(__dirname + '/build/');
  }).then(function() {
    return mkdir(__dirname + '/build/final/');
  }).then(function() {
    print(command);
    return run(command, { cwd: __dirname });
  }).then(function() {
    print.end("constructor ready in /build/final/");
  }).fin(function() {
    fs.unlinkSync(bundle);
  });
}

// default - full build
prepare_dependencies().then(build_constructor).catch(function(err) {
  print("Error: "+ err);
  process.exit(1);
});
