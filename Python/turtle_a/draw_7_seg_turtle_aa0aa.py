#!/usr/bin/python3

import time
import turtle

global start_time
print('hello')
# time.sleep(50)

def turt_setup():
    turtle.pensize(5)
    turtle.fillcolor('black')

    turtle.setposition(-440,290)


    # turtle.begin_poly()
    turtle.begin_fill()
    turtle.forward(700) # horizontal upper bound, drawn left to right
    turtle.right(90)

    turtle.forward(330) # turtle travels downwards, on the right edge of the new shape
    turtle.right(90)

    turtle.forward(700) # horiz. lower bound, drawn r to l
    turtle.right(90)

    turtle.forward(300)
    turtle.end_fill()

    # turtle.end_poly()

    turtle.right(90)

    turtle.pensize(24)
    turtle.color('red')
    turtle.speed(0)
    print('turtle.speed(0)')
    turtle.penup()

def trace_hstroke():
    turtle.left(100)
    turtle.forward(70)

def trace_extended_vstroke():
    turtle.forward(90)

def trace_vstroke():
    turtle.left(80)
    turtle.forward(90)

def repos_above():
    trace_hstroke()
    trace_vstroke()

def draw_eightbox():
    turtle.right(100) # fake
    trace_hstroke()
    trace_vstroke()
    trace_hstroke()
    trace_vstroke()

def draw_eight():
    turtle.pendown()
    draw_eightbox()
    repos_above()
    trace_hstroke()
    turtle.right(180)
    draw_eightbox()

def draw_first_eight():
    draw_eight()
    turtle.pendown()
    turtle.right(80) # 100 or 80 - or 180
    trace_vstroke()
    turtle.penup()
    print('turtle.pos() is ')
    print(turtle.pos())
    turtle.right(80)

def trace_digit_change_left():
    # reposition next digit to the left
    turtle.penup()
    # turtle.pendown() # debug

    turtle.forward(70)
    turtle.forward(70)

def draw_subsequent_digit_at_left():
    # trace_digit_change_left()
    turtle.pendown()
    draw_eight()


turt_setup()

turtle.setposition(30,30)
print('turtle.pos() is ')
print(turtle.pos())

draw_first_eight()

turtle.color('grey')
trace_digit_change_left()
turtle.right(180)

turtle.color('green')

draw_first_eight()

turtle.color('grey')
trace_digit_change_left()
turtle.right(180)

turtle.color('green')

draw_first_eight()

turtle.color('grey')
trace_digit_change_left()
turtle.right(180)

turtle.color('green')

draw_first_eight()

time.sleep(5)

while -1:
    time.sleep(0.001)

while -1:
    print(time.perf_counter())
    time.sleep(0.99897)

