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

var invalid = {
    '[a': /missing terminating \] for character class/,
    '(?<=.*)': /lookbehind assertion is not fixed length/,
    '(bc': /missing \)/
};

describe('FilterList', function () {
    describe('constructor', function () {
        it('should accept a valid list in the constructor', function () {
            var list = new FilterList(filters);
            assert.equal(list.length, filters.length);
        });

        it('should accept an empty list in the constructor', function () {
            var list = new FilterList([]);
            assert.equal(list.length, 0);
        });

        it('should allow no list in the constructor', function () {
            var list = new FilterList();
            assert.equal(list.length, 0);
        });

        it('should throw a TypeError if the argument is not an array', function () {
            assert.throws(function () {
                var list = new FilterList(2.3);
            }, /Argument to FilterList constructor must be an array/);
        });

        it('should throw a TypeError if any filter is not an object', function () {
            var f2 = Array.prototype.slice.call(filters);
            f2.push('abcdef');
            assert.throws(function () {
                var list = new FilterList(f2);
            }, /Filter at index 6 is not an object/);
        });

        it('should throw a TypeError if any filter is invalid', function () {
            var f2 = Array.prototype.slice.call(filters);
            f2.push({});
            assert.throws(function () {
                var list = new FilterList(f2);
            }, /Filter at index 6 is invalid/);
        });
    });

    describe('#quoteMeta', function () {
        for (var key in quoted) {
            it('should quote ' + key + ' correctly', function () {
                assert.equal(FilterList.quoteMeta(key), quoted[key]);
            });
        }
    });

    describe('#checkValidRegex', function () {
        Object.keys(invalid).forEach(function (key) {
            it('should raise an error for ' + key, function () {
                assert.throws(function () {
                    FilterList.checkValidRegex(key);
                }, invalid[key]);
            });
        });
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

    describe('#addFilter', function () {
        it('should throw an error if no arguments are given', function () {
            var list = new FilterList(filters);

            assert.throws(function () {
                list.addFilter();
            }, /addFilter expects 1 argument/);
        });

        it('should throw an error if the argument is not an object', function () {
            var list = new FilterList(filters);

            assert.throws(function () {
                list.addFilter(42);
            }, /Filter to add must be an object/);
        });

        it('should throw an error if the argument is not a valid filter', function () {
            var list = new FilterList(filters);

            assert.throws(function () {
                list.addFilter({ name: 'bad' });
            }, /Invalid filter/);
        });

        it('should add a valid filter', function () {
            var list = new FilterList(filters);

            var newf = {
                name: 'mmkay',
                source: '$',
                replace: 'mmkay',
                flags: '',
                active: true,
                filterlinks: false
            };

            list.addFilter(newf);

            var result = list.pack();
            assert.deepEqual(result[result.length - 1], newf);
            assert.equal(list.length, filters.length + 1);
        });

        it('should throw an error if a filter with the given name already exists', function () {
            var list = new FilterList(filters);

            var newf = {
                name: 'bold',
                source: '\\*\\*(.+?)\\*\\*',
                replace: '<strong>\\1</strong>',
                flags: '',
                active: true,
                filterlinks: false
            };

            assert.throws(function () {
                list.addFilter(newf);
            }, /Error: Filter 'bold' already exists.  Please choose a different name/);
        });
    });

    describe('#updateFilter', function () {
        it('should throw an error if no arguments are given', function () {
            var list = new FilterList(filters);

            assert.throws(function () {
                list.updateFilter();
            }, /updateFilter expects 1 argument/);
        });

        it('should throw an error if the argument is not an object', function () {
            var list = new FilterList(filters);

            assert.throws(function () {
                list.updateFilter(42);
            }, /Filter to be updated must be an object/);
        });

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
                }, new RegExp('Field ' + field + ' must be a string'));
            });

            ['active', 'filterlinks'].forEach(function (field) {
                var newf = { name: name };
                newf[field] = 42;

                var list = new FilterList(filters);
                assert.throws(function () {
                    list.updateFilter(newf);
                }, new RegExp('Field ' + field + ' must be a boolean'));
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

    describe('#removeFilter', function () {
        it('should throw an error if no arguments are given', function () {
            var list = new FilterList(filters);

            assert.throws(function () {
                list.removeFilter();
            }, /removeFilter expects 1 argument/);
        });

        it('should throw an error if the argument is not an object', function () {
            var list = new FilterList(filters);

            assert.throws(function () {
                list.removeFilter(42);
            }, /Filter to be removed must be an object/);
        });

        it('should return false if the filter does not exist', function () {
            var newf = { name: 'dne', source: 'asdf' };

            var list = new FilterList(filters);

            assert.equal(list.removeFilter(newf), false);
        });

        it('should remove filters correctly', function () {
            var list = new FilterList(filters);
            for (var i = 0; i < filters.length; i++) {
                assert.equal(list.removeFilter(filters[i]), true);
                assert.equal(list.length, filters.length - i - 1);
            }
        });
    });

    describe('#moveFilter', function () {
        it('should throw an error if no arguments are given', function () {
            var list = new FilterList(filters);

            assert.throws(function () {
                list.moveFilter();
            }, /moveFilter expects 2 arguments/);
        });

        it('should throw an error if either argument is not an integer', function () {
            var list = new FilterList(filters);

            assert.throws(function () {
                list.moveFilter(42.5, 1);
            }, /Arguments 'from' and 'to' must both be integers/);

            assert.throws(function () {
                list.moveFilter(1, "abc");
            }, /Arguments 'from' and 'to' must both be integers/);
        });

        it('should throw an error if an argument is out of range', function () {
            var list = new FilterList(filters);

            assert.throws(function () {
                list.moveFilter(-1, 0);
            }, /Argument out of range/);

            assert.throws(function () {
                list.moveFilter(filters.length, 0);
            }, /Argument out of range/);

            assert.throws(function () {
                list.moveFilter(1, filters.length);
            }, /Argument out of range/);

            assert.throws(function () {
                list.moveFilter(1, filters.length + 24);
            }, /Argument out of range/);
        });

        it('should move filters correctly', function () {
            // Pseudorandom sequence; hardcoded so that tests are deterministic
            var moves = [4, 1, 4, 0, 5, 5, 2, 3, 0, 3, 5, 4, 5, 4, 5, 3, 3, 4, 5, 0, 3];
            var list = new FilterList(filters);
            var prev = list.pack();

            for (var i = 0; i < 100; i++) {
                var from = moves[i % moves.length] % filters.length;
                var to = moves[(i+1) % moves.length] % filters.length;

                list.moveFilter(from, to);
                var next = list.pack();
                assert.deepEqual(next[to], prev[from]);
                assert.deepEqual(next[from], prev[to]);
                prev = next;
            }
        });
    });
});
