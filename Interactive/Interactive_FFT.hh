class aFft: public Action {
public:
    aFft() : Action(){
        description="Produce the real FFT of an <Image>";
        addname("fft");
        ninputs = 1;
        inputtypes[0] = COF_IMAGE;
    };
    
    void DoIt() {
        mystack->push(ComplexImage::fftreal((Image*)mystack->get()));
    }
};

//-----------------------------

class aInvfft: public Action {
public:
    aInvfft() : Action(){
        description="Get an Image from the inverse FFT of a <ComplexImage>";
        addname("invfft");
        ninputs = 1;
        inputtypes[0] = COF_COMPLEXIMAGE;
    };
    
    void DoIt() {
        mystack->push(((ComplexImage*)mystack->get())->inversefftreal());
    }
};

//-----------------------------

class aReal: public Action {
public:
    aReal() : Action(){
        description="Extract the real part of a <ComplexImage>";
        addname("real");
        ninputs = 1;
        inputtypes[0] = COF_COMPLEXIMAGE;
    };
    
    void DoIt() {
        mystack->push(((ComplexImage*)mystack->get())->real());
    }
};

//-----------------------------

class aImag: public Action {
public:
    aImag() : Action(){
        description="Extract the imaginary part of a <ComplexImage>";
        addname("imag");
        ninputs = 1;
        inputtypes[0] = COF_COMPLEXIMAGE;
    };
    
    void DoIt() {
        mystack->push(((ComplexImage*)mystack->get())->imag());
    }
};

//-----------------------------

class aMagv: public Action {
public:
    aMagv() : Action(){
        description="Extract the absolute value of a <ComplexImage>";
        addname("magv");
        ninputs = 1;
        inputtypes[0] = COF_COMPLEXIMAGE;
    };
    
    void DoIt() {
        mystack->push(((ComplexImage*)mystack->get())->magv());
    }
};

//-----------------------------