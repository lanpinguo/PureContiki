
import os
import os.path
import serial  
import time
from threading import Thread
from threading import Lock
from threading import Condition



FILTER=''.join([(len(repr(chr(x)))==3) and chr(x) or '.'
                for x in range(256)])

def hex_dump_buffer(src, length=16):
    """
    Convert src to a hex dump string and return the string
    @param src The source buffer
    @param length The number of bytes shown in each line
    @returns A string showing the hex dump
    """
    result = ["\n"]
    for i in range(0, len(src), length):		#Python3's range is Python2's xrange. There's no need to wrap an iter around it.
       chars = src[i:i+length]
       hex = ' '.join(["%02x" % x for x in chars])
       printable = ''.join(["%s" % ((x <= 127 and
                                     FILTER[x]) or '.') for x in chars])
       result.append("%04x  %-*s  %s\n" % (i, length*3, hex, printable))
    return ''.join(result)



default_timeout = 2
"""
Wait on a condition variable until the given function returns non-None or a timeout expires.
The condition variable must already be acquired.
The timeout value -1 means use the default timeout.
There is deliberately no support for an infinite timeout.
"""
def timed_wait(cv, fn, timeout=-1):
    if timeout == -1:
        timeout = default_timeout

    end_time = time.time() + timeout
    while True:
        val = fn()
        if val != None:
            return val

        remaining_time = end_time - time.time()
        cv.wait(remaining_time)

        if time.time() > end_time:
            return None

            
            
            
class HCHO_SENSOR(Thread):

    MAX_QUEUE_LEN = 2048
    
    def __init__(self,portNum,baudrate = 9600,timeout=None):
        Thread.__init__(self)
        self.com = serial.Serial(portNum,baudrate,timeout=timeout,interCharTimeout=0.5)  
        
        self.killed = False
        self.bytes_queues = []
        self.timeout = -1
        self.com.timeout = 1 
        # cvar serves double duty as a regular top level lock and
        # as a condition variable
        self.cvar = Condition()
        self.logFile = None
        log_filename_serial = '%s_' % portNum + time.strftime("%Y_%m_%d_%H_%M_%S.log",time.localtime(time.time()))
        try :
            self.logFile = open(log_filename_serial,'w') 
        except:
            raise
            
        self.start()
    
    
    
    
    def __del__(self):
        if self.logFile :
            self.logFile.close()
        #self.kill()
        pass

        
        
    def logFlush(self):
        if self.logFile:
            #print 'flush logFile'
            self.logFile.flush()
    
    
    
    def flushInput(self):
        with self.cvar: 
            self.bytes_queues = []
    
    def run(self):
        """
        Activity function for class
        """
        while not self.killed:

            # Enqueue characters
            # ch = self.com.read(1)
            # count = self.com.inWaiting()
            # ch += self.com.read(count)

            ch = self.com.readline()

            if ch == b'' or ch == None:
                continue
            #print(ch)
            #Todo
            # write char into log file
            if self.logFile :
                self.logFile.write(hex_dump_buffer(ch))
            if self.is_valid(ch,len(ch)):
                hcho_ug_per_m3 = ch[2] * 256 + ch[3]
                print("HCHO(mg per m3) : %2.3f " % (hcho_ug_per_m3/1000.0))
            else:
                continue

            with self.cvar:                
                queue = self.bytes_queues
                if len(queue) >= self.MAX_QUEUE_LEN:
                    # Queue full, throw away oldest
                    queue.pop(0)
                    #self.logger.debug("Discarding oldest packet to make room")
                queue.append(ch)
                self.cvar.notify_all()

        #self.logger.info("Thread exit")

    def is_valid(self,data,length):

        checksum = 0
        for i in range(1,length - 1):
            checksum += data[i]
        checksum = (~(checksum & 0xFF) + 1) & 0xFF
        
        if checksum != data[length - 1] :
            return False
        else:
            return True

    def cancel_read(self):
        """Cancel a blocking read operation, may be called from other thread"""
        self.com.cancel_read()

    def cancel_write(self):
        """Cancel a blocking write operation, may be called from other thread"""
        self.com.cancel_write()      
        
        
    def read(self,bytes = 1):
        res = []
        while True:
            with self.cvar: 
                queue = self.bytes_queues
                ret = timed_wait(self.cvar, lambda: True if len(queue) > 0 else None,timeout = self.timeout)
                if ret != None:
                    ch = queue.pop(0)
                    res.append(ch)
                else :
                    break
                if len(res) >= bytes:
                    break
        return ''.join(res)
        
        
        

       
    def readall(self):
        res = []
        while True:
            with self.cvar: 
                queue = self.bytes_queues
                ret = timed_wait(self.cvar, lambda: True if len(queue) > 0 else None,timeout = 0.5)
                if ret is None:
                    break;
                ch = queue.pop(0)
                res.append(ch)

        return ''.join(res)           
 
 
 
    def write(self,data):
        return self.com.write(data)
        
        
        
    def kill(self):
        """
        Stop the serial thread.
        """
        self.killed = True
        with self.cvar:
            self.cvar.notify_all()
        self.join() 
        
        
        
if __name__ == '__main__': 

    dtu = HCHO_SENSOR('COM3',9600,timeout=5)

    
    while True:
        cmd = input('>')
        
        if cmd == 'exit':
            break
        elif cmd == 'auto':
            dtu.write([0xFF,0x01,0x78,0x41,0x00,0x00,0x00,0x00,0x46])
        elif cmd == 'get':
            dtu.write([0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79])
        #dut.write(cmd + '\n')
        
        #str = dut.readall()
        #print(str)
        
    print('exit...')
    dtu.kill()
    
    
        