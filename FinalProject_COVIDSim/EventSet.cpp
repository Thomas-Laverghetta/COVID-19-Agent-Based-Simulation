#include "EventSet.h"

EventSet::Node::Node(){
    m_ea = nullptr;
    m_et = 0.0f;    
    m_next = nullptr;
}

EventSet::Node::Node(const Time& t, EventAction * ea){
    m_ea = ea;
    m_et = t;
    m_next = nullptr;
}

void EventSet::AddEvent(const Time& t, EventAction * ea){
      Node * new_node = new Node(t, ea);
    
    // Special case for the head end
    if (m_head == nullptr || (m_head == nullptr ? true : m_head->m_et >= new_node->m_et)) {  
        new_node->m_next = m_head;
        m_head = new_node;  
    }  
    else {  
        // Locate the node before the point of insertion 
        Node * current = m_head;  
        while (current->m_next!=nullptr &&  
            current->m_next->m_et < new_node->m_et)  
        {  
            current = current->m_next;  
        }  
        new_node->m_next = current->m_next;  
        current->m_next = new_node;  
    } 
    m_nodeCounter++;
}

unsigned int EventSet::EventSetSize()
{
    return m_nodeCounter;
}

EventAction * EventSet::GetEventAction(){
    // current head node
    Node * current = m_head;
    
    // swap nodes
    m_head = m_head->m_next;
    
    // get current event action
    EventAction * ea = current->m_ea;

    // deleting node from event set
    delete current;
    current = nullptr;
    m_nodeCounter--;

    return ea; 
}

Time EventSet::GetEventTime(){
    return m_head->m_et;
}