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
    assert(!bkk_decide(&s, 95, false));
    assert(!bkk_decide(&s, 95, false));
    assert(bkk_decide(&s, 95, false));
    for (int i = 0; i < 5; i++) assert(bkk_decide(&s, 10, false));
    assert(!bkk_decide(&s, 10, false));
    assert(!bkk_thermal(&s, 45999, 87000));
    assert(bkk_decide(&s, 0, true));
    assert(bkk_thermal(&s, 46000, 87000));
    assert(!bkk_decide(&s, 100, true));
    assert(bkk_thermal(&s, 43001, 80000));
    assert(bkk_thermal(&s, 43000, 80001));
    assert(!bkk_thermal(&s, 43000, 80000));
    assert(bkk_thermal(&s, 40000, 87001));
    assert(!bkk_decide(&s, 100, true));
    assert(!bkk_thermal(&s, 43000, 80000));
    assert(!bkk_decide(&s, 95, false));
    assert(!bkk_decide(&s, 95, false));
    assert(bkk_decide(&s, 95, false));
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
