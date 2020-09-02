painty_copts = ["-Wall", "-Werror"]

def painty_cc_library(
        name,
        srcs = None,
        hdrs = None,
        deps = None,
        tags = None,
        visibility = None,
        **kwargs):

    native.cc_library(
        name = name,
        srcs = srcs,
        hdrs = hdrs,
        deps = deps,
        tags = tags,
        visibility = visibility,
        copts = painty_copts,
        **kwargs
    )


def painty_cc_binary(name, srcs = None, hdrs = None, tags = None, visibility = None, **kwargs):

    native.cc_binary(name = name,
        srcs = srcs,
        hdrs = hdrs,
        tags = tags,
        copts = painty_copts,
        visibility = visibility,
        **kwargs,
    )



def painty_cc_test(name, srcs = None, hdrs = None, tags = None, **kwargs):

    native.cc_test(
        name = name,
        srcs = srcs,
        hdrs = hdrs,
        tags = tags,
        **kwargs
    )
