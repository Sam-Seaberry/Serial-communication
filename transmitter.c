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
        
        
        Buffer Txbuf; 
        
        Timeout tt;
    public:
        BufferedASCommunicator(PinName txp, PinName rxp, Direction commRole ): ASCommunicator(txp, rxp, 115200, senderReceiver), Txbuf(10){
            tt.attach(callback(this, &BufferedASCommunicator::Txcallback),0.1);
            }
       
        void Txcallback(void){
            Tx(Txbuf.get());
            tt.attach(callback(this, &BufferedASCommunicator::Txcallback),0.1);
            }
        
        void setvalue(float s){
            Txbuf.put(s);
            Txcallback();
            }
        
        
};
class Potentiometer {
    private:
        AnalogIn inputSignal;
        float VDD, currentSamplePC, currentSampleVolts;
    public:
        Potentiometer(PinName pin, float v) : inputSignal(pin), VDD(v) {
            currentSamplePC = 0.0;
            currentSampleVolts = 0.0;
            }
        float amplitudeVolts(void) { return (inputSignal.read()*VDD); }
        float amplitudePC(void) { return inputSignal.read(); }
        
        void sample(void) {
            currentSamplePC = inputSignal.read();
            currentSampleVolts = currentSamplePC*VDD;
        }
        
        float getCurrentSampleVolts(void) { 
            return currentSampleVolts; 
        }
        
        float getCurrentSamplePC(void) {
             return currentSamplePC; 
        }

};

class SamplePot : public Potentiometer{
    private:
        
        float freq;
        
        float cycle;
        Ticker tkr;
    public:
        SamplePot(PinName pin,float v, float f) : Potentiometer(pin,v) {
            freq= f;
            cycle = 1/f;
            
            tkr.attach(callback(this, &SamplePot::sample),cycle);
            }
};

int main() {
    char s =0;
    
    lcd.locate(0,15);
    Potentiometer pot(A0, 3.3);
    SamplePot* sample= new  SamplePot(A0,3.3,500);
    ASCommunicator a(PA_11, PA_12, 115200, sender);
    BufferedASCommunicator asc(PA_11, PA_12,  sender);
    
    while(true) {
        s = sample->getCurrentSamplePC()*100;
        asc.setvalue(s);
        
       
        lcd.locate(0,15);
        lcd.printf("Tx %d ", s);
       
    }
}
