#include "mbed.h"
#include "C12832.h"
 
//RawSerial USART(PA_11, PA_12);

C12832 lcd(D11, D13, D12, D7, D10);
enum Direction {sender, receiver, senderReceiver};
class Buffer {
     private:
        int* buf;
        int size, count;
        int in;
        int out;
    
    public:
        Buffer();
        Buffer (int s);
        ~Buffer();
        
        void put (int i){
            
            buf[in] = i;
            ++count;
            in=(in+1) % size;
        }
        int get(){
            int x;
            
            x = buf[out];
            buf[out]=0;
            --count;
            out=(out+1) % size;
            return (x);
            }
        bool isFull(void){
            return (count >= size);
            }
        bool isEmpty(void){
            return (count == 0);
            }
    
    
    
   
};
Buffer::Buffer(int s){
    size = s;
    in = 0;
    out = 0;
    count = 0;
    buf = new int[size];
    }
class ASCommunicator {
    private:
        RawSerial rs;
        int baudRate;
        Direction commType;
        //Buffer buf;
    public:
        //enum Direction {sender, receiver, senderReceiver}
        ASCommunicator(PinName txp, PinName rxp,int br, Direction commRole):  rs(txp, rxp){
            baudRate = br;
            rs.baud(baudRate);
            commType = commRole ;
            }
        
        void setBaudRate(float br){rs.baud(br);}
        
        float getBaudRate(void){return baudRate;}
        
        void setDirection(Direction commRole){commType = commRole;}
        
        Direction getDirection(void){return commType; }
        
        bool canTx(void){
            if(rs.writeable()){
                return 1;
                }
            else{
                return 0;
                }
            
            }// returns true if USART can send
        
        bool canRx(void){
            if(rs.readable()){
                return 1;
                }
            else {
                return 0;
                }
            
            } // returns true if USART can receive
        
        void Tx(float c){
            rs.putc(c); // send a char
         
        }
        float Rx(void){
            float s = rs.getc(); 
            return s;
            } // receive a char
};
class BufferedASCommunicator : public ASCommunicator {
    private:
        
        Buffer Rxbuf;
       
        Timeout to;
       
    public:
        BufferedASCommunicator(PinName txp, PinName rxp, Direction commRole ): ASCommunicator(txp, rxp, 115200, senderReceiver), Rxbuf(10){
            to.attach(callback(this, &BufferedASCommunicator::Rxcallback),0.1);
            }
        void Rxcallback(void){
            Rxbuf.put(Rx());
            to.attach(callback(this, &BufferedASCommunicator::Rxcallback),0.1);
            }
       
        float getvalue(void){
            Rxcallback();
            return Rxbuf.get();
            }
        
        
        
};
class PwmSpeaker {
    private:
        PwmOut outputSignal;
        float period; // in sec
        float frequency; // in Hz
        float minFreq, maxFreq; // min/max in Hz
        BufferedASCommunicator* buf;
        
        
    public:
        PwmSpeaker(PinName pin, float freq, float min, float max, BufferedASCommunicator* b ): outputSignal(pin), frequency(freq), minFreq(min), maxFreq(max), buf(b){
            outputSignal.write(.5);
            }

        void setFreq(float f){
           frequency = f;
          
         }
         void setmin(float min){
             minFreq = min;
             }
        void setmax(float max){
            maxFreq = max;
            }
        
         
        float getFreq(void){
         return frequency;
         }
         void updatefreq(void){
             frequency = (buf->getvalue()/100)*(maxFreq-minFreq)+minFreq;
             period = 1/frequency;
             outputSignal.period(period);
             }
         // other getters
       
};
int main() {
    
    lcd.locate(0,15);
    ASCommunicator a(PA_11, PA_12, 115200, receiver);
    BufferedASCommunicator* buf = new BufferedASCommunicator(PA_11, PA_12,  receiver);
    PwmSpeaker* speak = new PwmSpeaker(D6,1000,2000,9000, buf);
    while(true) {
        speak->updatefreq();
        lcd.locate(0,15);
        lcd.printf("Rx %f ", buf->getvalue());
        
       
    }
}

   


