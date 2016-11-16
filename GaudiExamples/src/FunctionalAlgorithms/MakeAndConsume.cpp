#include <GaudiKernel/AnyDataHandle.h>
#include <GaudiAlg/Producer.h>
#include <GaudiAlg/Consumer.h>
#include <GaudiAlg/Transformer.h>
#include <GaudiKernel/MsgStream.h>


namespace Gaudi { namespace Examples {

class IntDataProducer : public Gaudi::Functional::Producer<int()> {
public:
    IntDataProducer(const std::string& name, ISvcLocator* svcLoc)
             : Producer( name, svcLoc,
               KeyValue("OutputLocation", {"MyInt"})) {}

    int operator()() const override {
        info() << "executing IntDataProducer" << endmsg;
        return 7;
    }


};

DECLARE_COMPONENT(IntDataProducer)

class IntDataConsumer : public Gaudi::Functional::Consumer<void(const int&)> {
public:
    IntDataConsumer(const std::string& name, ISvcLocator* svcLoc)
             : Consumer( name, svcLoc,
               KeyValue("InputLocation", {"MyInt"})) {}

    void operator()(const int& input) const override {
        info() << "executing IntDataConsumer: " << input << endmsg;
    }


};

DECLARE_COMPONENT(IntDataConsumer)

class IntToFloatData : public Gaudi::Functional::Transformer<float(const int&)> {
public:
    IntToFloatData(const std::string& name, ISvcLocator* svcLoc)
            : Transformer(name, svcLoc,
              KeyValue("InputLocation", {"MyInt"}),
              KeyValue("OutputLocation", {"MyFloat"})) {}

    float operator() (const int& input) const override {
        info() << "Converting: " << input << endmsg;
        return float(input);
    }
};

DECLARE_COMPONENT(IntToFloatData)

class FloatDataConsumer : public Gaudi::Functional::Consumer<void(const float&)> {
public:
    FloatDataConsumer(const std::string& name, ISvcLocator* svcLoc)
             : Consumer( name, svcLoc,
               KeyValue("InputLocation", {"MyFloat"})) {}

    void operator()(const float& input) const override {
        info() << "executing FloatDataConsumer: " << input << endmsg;
    }
};

DECLARE_COMPONENT(FloatDataConsumer)
}}


