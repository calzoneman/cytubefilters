{
    "targets": [
        {
            "target_name": "cytubefilters",
            "sources": [
                "src/filter.cc",
                "src/filterlist.cc",
                "src/jsfilterlist.cc",
                "src/util.cc"
            ],
            "dependencies": [
                "deps/libpcre/libpcre.gyp:libpcre"
            ],
            "include_dirs": ["<!(node -e \"require('nan')\")", "/usr/include", "deps/libpcre"]
        }
    ]
}
