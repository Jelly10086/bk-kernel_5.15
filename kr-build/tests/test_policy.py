from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

BUILD = Path(__file__).resolve().parents[1]


class PolicyTests(unittest.TestCase):
    def test_actual_kernel_policy(self):
        source = r'''
#include <assert.h>
#include <stdbool.h>
#include "policy.h"
int main(void) {
    struct bkk_load_state s = {0};
    assert(!bkk_decide(&s, 95, false, false));
    assert(bkk_decide(&s, 95, false, false));
    for (int i = 0; i < 3; i++) assert(bkk_decide(&s, 10, false, false));
    assert(!bkk_decide(&s, 10, false, false));
    assert(!bkk_thermal(false, 45999, 87000));
    assert(bkk_decide(&s, 0, true, false));
    assert(bkk_thermal(false, 46000, 87000));
    assert(!bkk_decide(&s, 100, true, true));
    assert(bkk_thermal(true, 43001, 80000));
    assert(bkk_thermal(true, 43000, 80001));
    assert(!bkk_thermal(true, 43000, 80000));
    assert(bkk_thermal(false, 40000, 87001));
    assert(!bkk_decide(&s, 100, true, true));
    assert(!bkk_thermal(true, 43000, 80000));
    assert(!bkk_decide(&s, 95, false, false));
    assert(bkk_decide(&s, 95, false, false));

    struct bkk_load_state prime = {0};
    assert(!bkk_decide(&prime, 10, false, false));
    assert(s.boost && !prime.boost);
    assert(bkk_decide(&s, 60, false, false));
    assert(!bkk_decide(&prime, 95, false, false));
    assert(bkk_decide(&prime, 95, false, false));
    assert(!bkk_decide(&s, 100, false, true));
    assert(!s.high && !s.low && !s.boost);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp)
            (path / 'test.c').write_text(source)
            subprocess.run(['gcc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            '-I', str(BUILD / 'modules/bkk-perf'), str(path / 'test.c'),
                            '-o', str(path / 'test')], check=True)
            subprocess.run([str(path / 'test')], check=True)


if __name__ == '__main__':
    unittest.main()
