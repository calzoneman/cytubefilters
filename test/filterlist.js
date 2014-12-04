var assert = require('assert');
var FilterList = require('../index');

var filters = [
    {
        name: 'abcdef',
        source: 'abc',
        replace: 'def',
        flags: '',
        active: true,
        filterlinks: false
    },
    {
        name: 'ghijkl',
        source: 'ghi',
        replace: 'jkl',
        flags: 'ig',
        active: true,
        filterlinks: false
    },
    {
        name: 'letters and numbers',
        source: '[a-z]{2}(\\d+)',
        replace: 'the number is \\1',
        flags: 'g',
        active: true,
        filterlinks: false
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
});
