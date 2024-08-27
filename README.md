# FORCETTY

Some programs obnoxiously buffer their output if they detect that they aren't
outputing to a pty, in ways that `stdbuf -iL -oL` can't fix.  This is
particularly problematic when filtering the output of a log file where the
program streaming out the logs does this buffering, because log events may be
indefinitely delayed if there is little activity being logged (`aws logs tail
--follow` is one of these).

This tiny wrapper forks the specified program controlled by a pty, and writes
its output to the parent's stdout.

Handles SIGCHLD:
- Normal child exit: exits parent, propagating child's exit status.
- Child killed with signal: exits (status 0)
- Child dumped: exits (status 0)

On SIGINT, SIGTERM to parent: exits as expected.

## EXAMPLE

``` sh
forcetty aws logs tail /aws/lambda/foo --since 1m --follow --color off | fancy_logs_filter
```

## BUILDING

``` sh
make
sudo make install
```

See the top of the tiny `Makefile` for configuration knobs.

## LICENSE

This package is placed in the public domain: the author disclaims
copyright and liability to the extent allowed by law. For those
jurisdictions that limit an author’s ability to disclaim copyright this
package can be used under the terms of the CC0, BSD, or MIT licenses. No
attribution, permission or fees are required to use this for whatever
you like, commercial or otherwise, though I would urge its users to do
good and not evil to the world.
