#ifndef EVENTSET_H
#define EVENTSET_H

#include "EventAction.h"

typedef double Time;

/*
    This implementation will use a sorted linked list
*/
class EventSet
{
private:
    // Event Set data
    class Node{
    public:
        Node * m_next;
        EventAction * m_ea;
        Time m_et;

        Node();
        Node(const Time& t, EventAction * ea);
    };
    // head of linked list
    Node * m_head;

    // size of the event set
    unsigned int m_nodeCounter;
public:
    // saves event
    void AddEvent(const Time& t, EventAction * ea);

    // Returns size of Event Set
    unsigned int EventSetSize();

    // returns event with smallest time 
    EventAction * GetEventAction();

    // returns time with smallest time stamp and deletes event from set
    Time GetEventTime();

    // removes event
    //void RemoveEvent(EventAction * ea, Time t);

    // Defualt constructor
    inline EventSet(){
        m_head = nullptr;
        m_nodeCounter = 0;
    }

    // deletes all nodes 
    ~EventSet(){
        // Node * current = m_head;
        while (m_head != nullptr){
            Node * to_delete = m_head;
            m_head = m_head->m_next;
            delete to_delete;
            to_delete = nullptr; 
        }
        m_nodeCounter = 0;
    }
};
#endif