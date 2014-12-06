var assert = require('assert');
var FilterList = require('../index');

var filters = [
    {
        name: 'monospace',
        source: '`(.+?)`',
        replace: '<code>\\1</code>',
        flags: 'g',
        active: true,
        filterlinks: false
    },
    {
        name: 'bold',
        source: '\\*(.+?)\\*',
        replace: '<strong>\\1</strong>',
        flags: 'g',
        active: true,
        filterlinks: false
    },
    {
        name: 'italic',
        source: '_(.+?)_',
        replace: '<em>\\1</em>',
        flags: 'g',
        active: true,
        filterlinks: false
    },
    {
        name: 'strike',
        source: '~~(.+)~~',
        replace: '<s>\\1</s>',
        flags: 'g',
        active: true,
        filterlinks: false
    },
    {
        name: 'inline spoiler',
        source: '\\[sp\\](.*?)\\[\\/sp\\]',
        replace: '<span class="spoiler">\\1</span>',
        flags: 'ig',
        active: true,
        filterlinks: false
    },
    {
        name: '.pic',
        source: '(https?:\\/\\/.+?)\\.pic',
        replace: '<a href="\\1"><img src="\\1"></a>',
        flags: 'ig',
        active: true,
        filterlinks: true
    }
];

var quoted = {
    'x\\y': 'x\\\\y',
    'it cost $100': 'it\\ cost\\ \\$100',
    '3.14': '3\\.14',
    '1.5-2.0?': '1\\.5\\-2\\.0\\?'
};

describe('FilterList', function () {
    describe('constructor', function () {
        it('should accept a valid list in the constructor', function () {
            var list = new FilterList(filters);
        });

        it('should accept an empty list in the constructor', function () {
            var list = new FilterList([]);
        });

        it('should allow no list in the constructor', function () {
            var list = new FilterList();
        });

        it('should throw a TypeError if the argument is not an array', function () {
            assert.throws(function () {
                var list = new FilterList(2.3);
            }, TypeError, /Argument to FilterList constructor must be an array/);
        });

        it('should throw a TypeError if any filter is not an object', function () {
            var f2 = Array.prototype.slice.call(filters);
            f2.push('abcdef');
            assert.throws(function () {
                var list = new FilterList(f2);
            }, TypeError, /Filter at index 3 is not an object/);
        });

        it('should throw a TypeError if any filter is invalid', function () {
            var f2 = Array.prototype.slice.call(filters);
            f2.push({});
            assert.throws(function () {
                var list = new FilterList(f2);
            }, TypeError, /Filter at index 3 is invalid/);
        });
    });

    describe('#quoteMeta', function () {
        for (var key in quoted) {
            it('should quote ' + key + ' correctly', function () {
                assert.equal(FilterList.quoteMeta(key), quoted[key]);
            });
        }
    });

    describe('#pack', function () {
        it('should re-pack the original list correctly', function () {
            var f2 = filters.slice().map(function (f) {
                f.flags = f.flags.split('').sort().join('');
                return f;
            });
            var list = new FilterList(f2);
            var packed = list.pack().map(function (f) {
                f.flags = f.flags.split('').sort().join('');
                return f;
            });
            for (var i = 0; i < packed.length; i++) {
                assert.deepEqual(packed[i], f2[i]);
            }
        });
    });

    describe('#updateFilter', function () {
        it('should throw an error if the filter does not exist', function () {
            var newf = { name: 'dne', source: 'asdf' };

            var list = new FilterList(filters);

            assert.throws(function () {
                list.updateFilter(newf);
            }, /Filter to be updated does not exist/);
        });

        it('should throw an error if a field has the wrong value type', function () {
            var name = filters[0].name;
            ['source', 'replace', 'flags'].forEach(function (field) {
                var newf = { name: name };
                newf[field] = 42;

                var list = new FilterList(filters);
                assert.throws(function () {
                    list.updateFilter(newf);
                }, TypeError, new RegExp('Field ' + field + ' must be a string'));
            });

            ['active', 'filterlinks'].forEach(function (field) {
                var newf = { name: name };
                newf[field] = 42;

                var list = new FilterList(filters);
                assert.throws(function () {
                    list.updateFilter(newf);
                }, TypeError, new RegExp('Field ' + field + ' must be a boolean'));
            });
        });

        it('should update source correctly', function () {
            for (var i = 0; i < filters.length; i++) {
                var newf = {
                    name: filters[i].name,
                    source: filters[i].source + '|test'
                };

                var list = new FilterList(filters);
                list.updateFilter(newf);

                var result = list.pack();
                assert.equal(result.length, filters.length);
                assert.equal(result[i].source, newf.source);
            }
        });

        it('should update replacement correctly', function () {
            for (var i = 0; i < filters.length; i++) {
                var newf = {
                    name: filters[i].name,
                    replace: filters[i].replace + '|test'
                };

                var list = new FilterList(filters);
                list.updateFilter(newf);

                var result = list.pack();
                assert.equal(result.length, filters.length);
                assert.equal(result[i].replace, newf.replace);
            }
        });

        it('should update flags correctly', function () {
            for (var i = 0; i < filters.length; i++) {
                var newf = {
                    name: filters[i].name,
                    flags: 'gim'
                };

                var list = new FilterList(filters);
                list.updateFilter(newf);

                var result = list.pack();
                assert.equal(result.length, filters.length);
                assert.equal(result[i].flags, newf.flags);
            }
        });

        it('should update active correctly', function () {
            for (var i = 0; i < filters.length; i++) {
                var newf = {
                    name: filters[i].name,
                    active: !filters[i].active
                };

                var list = new FilterList(filters);
                list.updateFilter(newf);

                var result = list.pack();
                assert.equal(result.length, filters.length);
                assert.equal(result[i].active, newf.active);
            }
        });

        it('should update filterlinks correctly', function () {
            for (var i = 0; i < filters.length; i++) {
                var newf = {
                    name: filters[i].name,
                    filterlinks: !filters[i].filterlinks
                };

                var list = new FilterList(filters);
                list.updateFilter(newf);

                var result = list.pack();
                assert.equal(result.length, filters.length);
                assert.equal(result[i].filterlinks, newf.filterlinks);
            }
        });

        it('should return the updated filter', function () {
            for (var i = 0; i < filters.length; i++) {
                var newf = {
                    name: filters[i].name,
                    source: filters[i].source + '|test'
                };

                var list = new FilterList(filters);
                var updated = list.updateFilter(newf);

                var result = list.pack();
                assert.deepEqual(updated, result[i]);
            }
        });
    });
});
