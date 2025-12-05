# MirrorBooking Alpha

A command-line appointment booking system designed for use in barbershop(s). Quick, efficient booking with single-line commands and persistent storage. The idea is to ditch mobile apps cluttered with menus and inputting with fingers, and to instead use a shell-like program to quickly type out commands followed me parameters. Along with quick-glance and legible information as needed. The ultimate end goal is to have it serve as a program or module compatible with a Smart Mirror running on raspberry OS. Even further out - Have it be less niche and have easy configuration tools to cater to much broader needs/industries.

## Features

- **Shell-like interface** - Single-line commands for fast booking
- **Service types** - Hair (30 min), Beard (15 min), Full service (45 min)
- **Smart scheduling** - Use `next` to auto-schedule the next available slot
- **Overlap detection** - Prevents double-booking
- **Admin override** - Book after-hours appointments (6pm-10pm)
- **Multiple views** - Daily and weekly schedule displays
- **Persistent storage** - All appointments saved to file

## Commands

```
add <name> <time> <service> [date]    Add appointment (date defaults to today)
del <name> <time>                      Delete appointment
reschedule <name> <time> <new-time>    Reschedule appointment
display [daily|weekly] [next|prev]     Show schedule
help                                   Show detailed help
exit                                   Save and exit
```

## Examples

```
add Henry 10am hair
add John next beard
add Jane 2pm full 2025-12-05
display weekly
reschedule Henry 10am 2pm
```

## Compilation

```powershell
clang++ main.cpp -o MirrorBooking.exe 
```

## To-Do List
- [ ] Refactor
- [ ] Monthly display view
- [ ] Color coding for appointments/services
- [ ] Smarter/flexible input field validation
- [ ] Tab-completion for commands
- [ ] Initialize Config for broader application


## Video
https://drive.google.com/file/d/174geqUR7Nygzuc36DWMWA8NYCiRaxX_b/view?usp=sharing