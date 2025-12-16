#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <string>

using namespace std;

struct DataPacket
{
    int packet_id;
    string packet_content;

    DataPacket(int id_num, const string& content_str)
        : packet_id(id_num), packet_content(content_str) {}
};

class MessageQueue
{
private:
    mutex queue_lock;
    condition_variable data_produced;
    condition_variable data_consumed;
    DataPacket* stored_packet = nullptr;
    bool packet_ready = false;

public:
    void send_packet(DataPacket* new_packet)
    {
        unique_lock<mutex> lock_guard(queue_lock);

        data_consumed.wait(lock_guard, [this]()
        {
            return !packet_ready;
        });

        stored_packet = new_packet;
        packet_ready = true;

        cout << "[Sender] Sending packet #" << new_packet->packet_id << endl;

        data_produced.notify_one();

        lock_guard.unlock();
    }

    DataPacket* receive_packet()
    {
        unique_lock<mutex> lock_guard(queue_lock);

        data_produced.wait(lock_guard, [this]()
        {
            return packet_ready;
        });

        DataPacket* received_packet = stored_packet;
        packet_ready = false;
        stored_packet = nullptr;

        cout << "[Receiver] Received packet #" << received_packet->packet_id << endl;

        data_consumed.notify_one();

        lock_guard.unlock();

        return received_packet;
    }
};

MessageQueue message_queue;

void sender_process()
{
    for (int packet_num = 1; packet_num <= 10; packet_num++)
    {
        this_thread::sleep_for(chrono::seconds(1));

        string data_content = "Data block #" + to_string(packet_num);
        DataPacket* new_packet = new DataPacket(packet_num, data_content);

        message_queue.send_packet(new_packet);
    }
}

void receiver_process()
{
    for (int packet_num = 1; packet_num <= 10; packet_num++)
    {
        DataPacket* received_packet = message_queue.receive_packet();

        this_thread::sleep_for(chrono::milliseconds(100));

        delete received_packet;
    }
}

int main()
{
    cout << "Starting synchronized message system" << endl;
    cout << "Processing 10 data packets..." << endl << endl;

    thread sender_thread(sender_process);
    thread receiver_thread(receiver_process);

    sender_thread.join();
    receiver_thread.join();

    cout << endl << "Data transmission completed" << endl;

    return 0;
}