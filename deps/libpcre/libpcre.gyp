{
    "targets": [
        {
            "target_name": "libpcre",
            "include_dirs": [
                "."
            ],
            "defines": [
                "PCRE_STATIC",
                "HAVE_CONFIG_H"
            ],
            "type": "static_library",
            "sources": [
                 "pcre_byte_order.c",
                 "pcre_chartables.c",
                 "pcre_compile.c",
                 "pcre_config.c",
                 "pcre_dfa_exec.c",
                 "pcre_exec.c",
                 "pcre_fullinfo.c",
                 "pcre_get.c",
                 "pcre_globals.c",
                 "pcre_jit_compile.c",
                 "pcre_maketables.c",
                 "pcre_newline.c",
                 "pcre_ord2utf8.c",
                 "pcre_refcount.c",
                 "pcre_stringpiece.cc",
                 "pcre_string_utils.c",
                 "pcre_study.c",
                 "pcre_tables.c",
                 "pcre_ucd.c",
                 "pcre_valid_utf8.c",
                 "pcre_version.c",
                 "pcre_xclass.c",
                 "pcrecpp.cc"
            ],
            "conditions": [
                [ "OS=='linux'", {
                    "include_dirs": ["linux"]
                }],
                [ "OS=='freebsd'", {
                    "include_dirs": ["freebsd"]
                }]
            ]
        }
    ]
}
