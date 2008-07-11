import pygtk
pygtk.require('2.0')
import gtk.glade
import socket
import time
import struct
import sys
import thread
gtk.gdk.threads_init()

RO = 0
RW = 1
MODE = RW

g_cursor_position = 0
maxtime = 1000
currfile = ""

readers = 0

def set_read_mode():
    for w in [slider,button]:
        w.set_property("visible",True)
def set_write_mode():
    for w in [slider,button]:
        w.set_property("visible",False)

def set_main_title():
    w = xml.get_widget("window1")
    text = "Time Based Text"
    if currfile:
        text = currfile + " - " + text
    else:
        text = "newfile" + " - " + text
    w.set_title(text)

def set_current_file(newfile):
    global currfile
    currfile = newfile
    set_main_title()

###################################################
# Menu Callbacks
def file_new(*args):
    global last_time
    global f
    set_current_file("")
    set_main_title()
    post("new file")
    set_write_mode()
    text_buffer.set_text("")
    last_time = time.time()
    f = open("/tmp/.output.tbt","wb")

def do_open_file(widget,filesel):
    global file
    post("open:"+filesel.get_filename())
    file = filesel.get_filename()
    filesel.destroy()
    set_read_mode()
    text_buffer.set_text("")
    thread.start_new_thread(printer_thread,(file,))

def file_open(*args):
    filesel = gtk.FileSelection(title="select a tbt file")
    filter = gtk.FileFilter()
    filter.add_pattern("*.tbt")
    filesel.complete("*.tbt")
    #filesel.file_list.set_filter(filter)
    filesel.ok_button.connect("clicked", do_open_file,filesel)
    filesel.cancel_button.connect("clicked", lambda w: filesel.destroy())
    filesel.show()

def do_save():
    global f
    # write from /tmp/.output.tbt to currfile
    f.close()
    a = open("/tmp/.output.tbt","rb")
    b = open(currfile,"w")
    b.write(a.read())
    b.close()
    a.close()
    f = open("/tmp/.output.txt","wab")
    post("file saved: "+currfile)

def do_save_file(w,filesel):
    set_current_file(filesel.get_filename())
    do_save()
    filesel.destroy()

def do_select_and_save():
    filesel = gtk.FileSelection(title="select a tbt file")
    filesel.ok_button.connect("clicked", do_save_file,filesel)
    filesel.cancel_button.connect("clicked", lambda w: filesel.destroy())
    filesel.show()
 
def file_save(*args):
    global currfile
    if not currfile:
        do_select_and_save()
    else:
        do_save()

def file_save_as(*args):
    currfile = do_select_and_save()

def close_event(*args):
    print "close event"
def dialog_close(widget,*args):
    widget.hide()
    return 1
def show_about(*args):
    win = xml.get_widget("aboutdialog1")
    win.show()

###################################################
# TextArea/Buffer Callbacks

def insert_text(textbuffer,iter,text,length):
    #print "a",textbuffer.get_property("cursor-position")
    #print "INSERT",text
    for t in text.decode("utf-8"):
        write_tbt(t.encode('utf-8'))

def delete_text(*args):
    #print "DELETE_TEXT",args
    #print "a",textbuffer.get_property("cursor-position")
    write_tbt("\127")
def delete_range(*args):
    print "DELETE_RANGE",args
    #print "a",textbuffer.get_property("cursor-position")
    #write_tbt("\127")


def backspace_text(*args):
    global g_cursor_position
    #print "BACKSPACE",args,g_cursor_position
    #print "a",textbuffer.get_property("cursor-position")
    write_tbt("\b")
    g_cursor_position-=2
    #print "BACKSPACE END",args,g_cursor_position

key_up = unichr(257).encode("utf-8")
key_down = unichr(256).encode("utf-8")
key_left = unichr(257).encode("utf-8")
key_right = unichr(258).encode("utf-8")

def cursor_position(widget,prop):
    global g_cursor_position
    #print "CURSOR_POS",g_cursor_position,widget.get_property(prop.name)
    pos = widget.get_property(prop.name)
    if pos != g_cursor_position:
        if pos < g_cursor_position:
            offset = g_cursor_position-pos
            while(offset):
                write_tbt(key_left)
                offset-=1
        elif pos > g_cursor_position:
            offset = pos-g_cursor_position
            while(offset):
                write_tbt(key_right)
                offset-=1
        #print "cursor_position",pos
        g_cursor_position = pos
    #print "CURSOR_POS END"

###################################################
# tbt read/write 

def get_tbt(data,idx):
    fmt_size = struct.calcsize("ccccccccQ")
    linedata = struct.unpack_from("ccccccccQ",data,idx*fmt_size)
    chardata = linedata[:-1]
    chardata = filter(lambda s: s!="\x00",chardata)
    chardata = "".join(chardata)
    nexttime = linedata[-1]
    return nexttime,chardata

def write_tbt(text):
    global g_cursor_position,last_time
    g_cursor_position+=1
    if MODE != RW:
        return
    currtime = int((time.time()-last_time)*1000)
    last_time = time.time()
    #print "write",currtime,text
    if text == "\n":
        text = "\r"
    if currtime > maxtime:
        currtime = maxtime
    utf8_text = text.decode("utf-8").encode('utf-8')
    format = "%ds%dxQ"%(len(utf8_text),8-len(utf8_text))
    data = struct.pack(format,utf8_text,currtime)
    #print format,data,struct.unpack("QQ",data)
    f.write(data)

###################################################
# thread
def printer_thread(filename):
    global readers
    readers+=1
    thisreader = readers
    print "reader!",filename
    f = open(filename,"rb")
    data = f.read()
    idx = 0
    while True:
        currspeed = slider.get_value()
        if currspeed == 0:
            currspeed = 1.0
        try:
            nexttime,nextchar = get_tbt(data,idx)
        except:
            print "finished reading"
            break
        time.sleep(nexttime/(1000.0*currspeed))
        if not thisreader == readers:
            # only one reader thread must live...
            break;
        #print "NEXTCHAR",len(nextchar),nextchar
        gtk.gdk.threads_enter()
        if nextchar == key_left:
            iter = text_buffer.get_iter_at_offset(text_buffer.get_property("cursor-position")-1)
            text_buffer.place_cursor(iter)
        elif nextchar == key_right:
            iter = text_buffer.get_iter_at_offset(text_buffer.get_property("cursor-position")+1)
            text_buffer.place_cursor(iter)
        elif nextchar == "\b":
            iter = text_buffer.get_iter_at_offset(text_buffer.get_property("cursor-position"))
            text_buffer.backspace(iter,False,True)
        else:    
            text_buffer.insert_at_cursor(nextchar)
        gtk.gdk.threads_leave()
        idx+=1
    f.close()

###################################################
# interface setup

xml = gtk.glade.XML("gtbt.glade")
textview = xml.get_widget("textview1")
slider = xml.get_widget("hscale1")
button = xml.get_widget("button1")
statusbar = xml.get_widget("statusbar1")
ctx_id = statusbar.get_context_id("tbt")
def post(text):
    statusbar.push(ctx_id,text)
text_buffer = textview.get_buffer()
text_buffer.notify("cursor-position")
text_buffer.connect("insert-text",insert_text)
text_buffer.connect("delete-range",delete_range)
text_buffer.connect("notify",cursor_position)

file = "/tmp/.output.tbt"
if len(sys.argv)>1 and sys.argv[-1].endswith("tbt"):
    file = sys.argv[-1]

if "-p" in sys.argv:
    set_read_mode()
    MODE = RO
    # play mode
    textview.set_editable(False)
    thread.start_new_thread(printer_thread,(file,))
    post("reading "+file)
else:
    set_write_mode()
    f = open(file,"wb")
    last_time = time.time()
    post("writing "+file)

###################################################
# random stuff

#def send_stuff(path,val):
#    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#    sock.connect(('',8666))
#    sock.send("%s %s\n"%(path,val))
#    sock.close()

def slider_moves(slider):
    val = slider.get_value()
    #send_stuff("/foo/bar",val)

def button_clicked(button):
    print "restart"
    text_buffer.set_text("")
    thread.start_new_thread(printer_thread,(file,))
    #send_stuff("/foo/bar",0)

def ondestroy(*args):
    try:
        f.close()
    except:
        pass
    gtk.main_quit()

###################################################
# Connect and Run!

xml.signal_autoconnect(locals())

gtk.main()

