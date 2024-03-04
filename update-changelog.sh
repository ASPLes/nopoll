#!/bin/bash

gitlog-to-changelog  | sed  's/nopoll: *//g' > ChangeLog
