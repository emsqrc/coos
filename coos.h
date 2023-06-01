


#define cLENGTH 4 //степень двойки
#define cMOD (cLENGTH-1)  //количество бит нужное для представления cLENGTH 
#define cUSERTYPE int

#define cTIMERLENGTH 3
#define cGetMilliseconds() millis()
#define cSLEEPTIME 1


class Tcoosfifo{
  cUSERTYPE byf[cLENGTH];
  word pb,pe;
  inline void Inc(word &val){
    val++;
    val &=cMOD;
  }
  inline void Deinc(word &val){
    val--;
    val &=cMOD;
  }
  public:
  Tcoosfifo(void){
    pb = 0;
    pe = 0; 
    for(int i=0; i<cLENGTH; i++){
      byf[i] = 0;
    } 
  }
  word Count(void){
    return (pe-pb)&cMOD;
  }
  bool Push(cUSERTYPE val){
    if(Count()<cLENGTH-1){
      byf[pe] = val;
      Inc(pe);
      return true;
    }else return false;
  }
  cUSERTYPE Pop(void){
    if(Count()>0){
      cUSERTYPE temp = byf[pb];
      Inc(pb);
      return temp;
    }
    else return 0;
  }
  cUSERTYPE Read(word i){
    if(Count()>0){
      return byf[(pb+i)&cMOD];
    }
    else return 0;
  }
};

class TcoosTimer{
    long tmS[cTIMERLENGTH];
    long tmC[cTIMERLENGTH];
    volatile void *tmcallback[cTIMERLENGTH];
    word countmaxindex;
public:
    TcoosTimer(void){
        for(int i=0; i<cTIMERLENGTH; i++){
            tmcallback[i] = 0;
        }
        countmaxindex = 0;
    }
    bool Add(long _time, void *call){
        bool r = false;
        for(uint16_t i = 0; i<cTIMERLENGTH; i++){
            noInterrupts();
            if(tmcallback[i]==0){
                tmS[i] = cGetMilliseconds();
                tmC[i] = _time;
                tmcallback[i] = call;
                if(i>countmaxindex) countmaxindex = i;
                r = true;
                interrupts();
                break;
            }
            interrupts();
        }
        return r;
    }
     cUSERTYPE Math(void){
        cUSERTYPE temp = 0;
            for(uint16_t i = 0; i<=countmaxindex; i++){
                if(tmcallback[i]>0){
                    if((long)(cGetMilliseconds()-tmS[i])>=(long)tmC[i]){
                        temp = (int)tmcallback[i];
                        tmcallback[i] = 0;
                        if(countmaxindex==i){
                            while((tmcallback[countmaxindex]==0)&&(countmaxindex>0)) countmaxindex--;
                        }
                        break;
                    }
                }
            }
        return temp;
    }
};

class Tcoos{
    Tcoosfifo fifo;
    TcoosTimer timers;
    long oldMathTime;
    void (*Pidle)();
    void (*Perror)();
    void idle(void){
      if((cUSERTYPE)Pidle>0){
         Pidle();
      }
    }
    void RunFunc(void){
        void (*temp)();
        temp = (void(*)(void)) fifo.Pop();
        if((cUSERTYPE)temp>0){
            temp();
        }else{
            idle();
        }
    }
public:
    Tcoos(void){
      Pidle = 0;
      Perror = 0;
      oldMathTime = cGetMilliseconds();
    }
    void SetErrorsCallback(void (*callback)()){
        Perror =callback;
    }
    void Add(void (*callback)(), long _time){
        if(!timers.Add(_time, (void *)callback)){
            Perror();
        }
    }
    void Add(void (*callback)()){
        noInterrupts();
        if(!fifo.Push((cUSERTYPE) callback)) this->Add(callback,cSLEEPTIME);
        interrupts();
    }
    void Run(void){
        while(true){
            void *temp;
            RunFunc(); 
            if((long)cGetMilliseconds()-oldMathTime>=1){
              temp = (void *)timers.Math();
              while(temp>0){
                  Add((void (*)())temp);
                  temp = (void *)timers.Math();
              }
            oldMathTime = cGetMilliseconds();
            }
        }
    }
    void SetIdle(void (*call)()){
      Pidle = call;
    }
};
