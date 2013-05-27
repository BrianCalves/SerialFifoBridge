//
// Relay data between a serial port and a pair of named pipes (FIFOs).
// The pipes must be created independently, such as by the mkfifo(1) command.
// This program does not automatically re-open pipes following SIGHUP.
//

#include <errno.h>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <stdint.h>
#include <sstream>
#include <string>
#include <sysexits.h>
#include <sys/select.h>
#include <termios.h>

using std::auto_ptr;
using std::cerr;
using std::cin;
using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::string;
using std::ostringstream;

typedef uint8_t octet;

int main(int argc, char** argv, char** envp)
    {
    if (argc != 4)
        {
        cerr << "usage: " << argv[0] << " <serial-device> <s2c-fifo> <c2s-fifo>" << endl
             << "       Relay data between a serial port and a client via a pair of named pipes (FIFOs)." << endl
             << "       The pipes must be created independently, such as by mkfifo(1)." << endl;
        return EX_USAGE;
        }

    string deviceName(argv[1]);
    string serialToClientName(argv[2]);
    string clientToSerialName(argv[3]);

    int serial = open(deviceName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK | O_EXLOCK);
    if (serial < 0)
        {
        ostringstream tmp;
        tmp << "Unable to open serial device " << deviceName.c_str();
        perror(tmp.str().c_str());
        return EX_NOINPUT;
        }

    struct termios portSettings;
    bzero(&portSettings, sizeof(portSettings));

    cfsetispeed(&portSettings, B9600);
    cfsetospeed(&portSettings, B9600);
		
    portSettings.c_cflag &= ~CSIZE;      // character size mask 
    portSettings.c_cflag &= ~CS8;        // 8 bits 
    portSettings.c_cflag &= ~CSTOPB;     // send 2 stop bits 
    portSettings.c_cflag &= ~CREAD;      // enable receiver 
    portSettings.c_cflag &= ~PARENB;     // parity enable 
    portSettings.c_cflag &= ~PARODD;     // odd parity, else even 
    portSettings.c_cflag &= ~HUPCL;      // hang up on last close 
    portSettings.c_cflag &= ~CLOCAL;     // ignore modem status lines 
    portSettings.c_cflag &= ~CCTS_OFLOW; // CTS flow control of output 
    portSettings.c_cflag &= ~CRTSCTS;    // same as CCTS_OFLOW 
    portSettings.c_cflag &= ~CRTS_IFLOW; // RTS flow control of input 
    portSettings.c_cflag &= ~MDMBUF;     // flow control output via Carrier 

    portSettings.c_cflag |= CREAD;
    portSettings.c_cflag |= CS8;
    portSettings.c_cflag |= CLOCAL;
    
    portSettings.c_iflag &= ~IGNBRK;    // ignore BREAK condition 
    portSettings.c_iflag &= ~BRKINT;    // map BREAK to SIGINTR 
    portSettings.c_iflag &= ~IGNPAR;    // ignore (discard) parity errors 
    portSettings.c_iflag &= ~PARMRK;    // mark parity and framing errors 
    portSettings.c_iflag &= ~INPCK;     // enable checking of parity errors 
    portSettings.c_iflag &= ~ISTRIP;    // strip 8th bit off chars 
    portSettings.c_iflag &= ~INLCR;     // map NL into CR 
    portSettings.c_iflag &= ~IGNCR;     // ignore CR 
    portSettings.c_iflag &= ~ICRNL;     // map CR to NL (ala CRMOD) 
    portSettings.c_iflag &= ~IXON;      // enable output flow control 
    portSettings.c_iflag &= ~IXOFF;     // enable input flow control 
    portSettings.c_iflag &= ~IXANY;     // any char will restart after stop 
    portSettings.c_iflag &= ~IMAXBEL;   // ring bell on input queue full 
 // portSettings.c_iflag &= ~IUCLC;     // translate upper case to lower case 
    
    portSettings.c_oflag &= ~OPOST;   // enable following output processing 
    portSettings.c_oflag &= ~ONLCR;   // map NL to CR-NL (ala CRMOD) 
    portSettings.c_oflag &= ~OXTABS;  // expand tabs to spaces 
    portSettings.c_oflag &= ~ONOEOT;  // discard EOT's `^D' on output) 
    portSettings.c_oflag &= ~OCRNL;   // map CR to NL 
 // portSettings.c_oflag &= ~OLCUC;   // translate lower case to upper case 
    portSettings.c_oflag &= ~ONOCR;   // No CR output at column 0 
    portSettings.c_oflag &= ~ONLRET;  // NL performs CR function 
           
    portSettings.c_lflag &= ~ECHOKE;      // visual erase for line kill 
    portSettings.c_lflag &= ~ECHOE;       // visually erase chars 
    portSettings.c_lflag &= ~ECHO;        // enable echoing 
    portSettings.c_lflag &= ~ECHONL;      // echo NL even if ECHO is off 
    portSettings.c_lflag &= ~ECHOPRT;     // visual erase mode for hardcopy 
    portSettings.c_lflag &= ~ECHOCTL;     // echo control chars as ^(Char) 
    portSettings.c_lflag &= ~ISIG;        // enable signals INTR, QUIT, [D]SUSP 
    portSettings.c_lflag &= ~ICANON;      // canonicalize input lines 
    portSettings.c_lflag &= ~ALTWERASE;   // use alternate WERASE algorithm 
    portSettings.c_lflag &= ~IEXTEN;      // enable DISCARD and LNEXT 
    portSettings.c_lflag &= ~EXTPROC;     // external processing 
    portSettings.c_lflag &= ~TOSTOP;      // stop background jobs from output 
    portSettings.c_lflag &= ~FLUSHO;      // output being flushed (state) 
    portSettings.c_lflag &= ~NOKERNINFO;  // no kernel output from VSTATUS 
    portSettings.c_lflag &= ~PENDIN;      // XXX retype pending input (state) 
    portSettings.c_lflag &= ~NOFLSH;      // don't flush after interrupt 		
    
		
    if (tcsetattr(serial, TCSANOW, &portSettings) < 0)
        {
        cerr << "Unable to set terminal attributes for " << deviceName.c_str() << endl;
        return EX_IOERR;        
        }

    /*
    if (tcflush(serial, TCIOFLUSH) < 0)
        {
        char *x = strerror(errno);
        cerr << "Unable to flush " << deviceName.c_str() << ": " << x << endl;
        return EX_IOERR;        
        }
    */
   
    int c2s = open(clientToSerialName.c_str(), O_RDONLY | O_NONBLOCK);
    if (c2s < 0)
        {
        ostringstream tmp;
        tmp << "Unable to open FIFO " << clientToSerialName.c_str();
        perror(tmp.str().c_str());
        return EX_NOINPUT;
        }

    int s2c = open(serialToClientName.c_str(), O_WRONLY /* | O_NONBLOCK */);
    if (s2c < 0)
        {
        ostringstream tmp;
        tmp << "Unable to open FIFO " << serialToClientName.c_str();
        perror(tmp.str().c_str());
        return EX_NOINPUT;
        }

    long timeOutInSeconds = 1;

    fd_set readSet;
    fd_set writeSet;
    fd_set errorSet;

    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_ZERO(&errorSet);

    FD_SET(serial, &readSet);
    FD_SET(serial, &writeSet);
    FD_SET(serial, &errorSet);

    FD_SET(s2c, &writeSet);
    FD_SET(s2c, &errorSet);

    FD_SET(c2s, &readSet);
    FD_SET(s2c, &errorSet);

    int largestFileDescriptorInUse = 0;

    if (largestFileDescriptorInUse < serial)
        largestFileDescriptorInUse = serial;

    if (largestFileDescriptorInUse < s2c)
        largestFileDescriptorInUse = s2c;

    if (largestFileDescriptorInUse < c2s)
        largestFileDescriptorInUse = c2s;
        
    while (true)
        {

        struct timeval timeOut;
        timeOut.tv_sec = timeOutInSeconds;
        timeOut.tv_usec = 0;

        fd_set temporaryReadSet;
        fd_set temporaryWriteSet;
        fd_set temporaryErrorSet;

        FD_ZERO(&temporaryReadSet);
        FD_ZERO(&temporaryWriteSet);
        FD_ZERO(&temporaryErrorSet);

        FD_COPY(&readSet, &temporaryReadSet);
        FD_COPY(&writeSet, &temporaryWriteSet);
        FD_COPY(&errorSet, &temporaryErrorSet);

        int readyCount = select(
            largestFileDescriptorInUse + 1,
            &temporaryReadSet,
            &temporaryWriteSet,
            &temporaryErrorSet,
            &timeOut);

        if (readyCount < 0)
            {
            if (errno == EINTR)
                continue;

            perror("select() failed");
            return EX_SOFTWARE;
            }

        if (readyCount == 0)
            {
            // Do nothing. If there is no data to relay, this program
            // does not have any processing to perform. Instead of idly
            // continuing, this program could have been implemented
            // such that select(2) simply blocks indefinitely.

            continue;
            }
        
        if (FD_ISSET(serial, &temporaryErrorSet))
            {
            perror("select() indicated error on serial port");
            return EX_IOERR;
            }

        if (FD_ISSET(s2c, &temporaryErrorSet))
            {
            perror("select() indicated error on serial-to-client pipe");
            return EX_IOERR;
            }

        if (FD_ISSET(c2s, &temporaryErrorSet))
            {
            perror("select() indicated error on client-to-serial pipe");
            return EX_IOERR;
            }

        if (FD_ISSET(serial, &temporaryReadSet) &&
            FD_ISSET(s2c, &temporaryWriteSet))
            {
            // Copy from serial port to client.
            // Copy one octet at a time, thus trading efficiency for simplicity.

            octet buffer;

            ssize_t readCount = read(serial, &buffer, 1);
            if (readCount < 0)
                {
                perror("read() from serial port");
                return EX_IOERR;
                }
            if (readCount == 0) // end-of-file
                {
                cout << "Read end-of-file from serial device." << endl;
                return EX_OK;
                }

            ssize_t writeCount = write(s2c, &buffer, 1);
            if (writeCount < 0)
                {
                perror("write() to client");
                return EX_IOERR;
                }
            if (writeCount == 0)
                {
                cerr << "write() to client failed." << endl;
                return EX_IOERR;
                }
            if (writeCount > 1)
                {
                cerr << "write() to client excess." << endl;
                return EX_OSERR;
                }

            if (isprint(buffer))
                cout << "serial: '" << char(buffer) << "'" << endl;
            else
                cout << "serial: " << hex << "0x" << uint32_t(buffer) << dec << endl;
            }

        if (FD_ISSET(c2s, &temporaryReadSet) &&
            FD_ISSET(serial, &temporaryWriteSet))
            {
            // Copy from client to serial port.
            // Copy one octet at a time, thus trading efficiency for simplicity.

            octet buffer;

            ssize_t readCount = read(c2s, &buffer, 1);
            if (readCount < 0)
                {
                perror("read() from client");
                return EX_IOERR;
                }
            if (readCount == 0) // end-of-file
                {
                cout << "Read end-of-file from client." << endl;
                return EX_OK;
                }

            ssize_t writeCount = write(serial, &buffer, 1);
            if (writeCount < 0)
                {
                perror("write() to serial port");
                return EX_IOERR;
                }
            if (writeCount == 0)
                {
                cerr << "write() to serial port failed." << endl;
                return EX_IOERR;
                }
            if (writeCount > 1)
                {
                cerr << "write() to serial port excess." << endl;
                return EX_OSERR;
                }

            if (isprint(buffer))
                cout << "client: '" << char(buffer) << "'" << endl;
            else
                cout << "client: " << hex << "0x" << uint32_t(buffer) << dec << endl;
            }

        }

    close(c2s);
    close(s2c);
    close(serial);

    return EX_OK;

    }


