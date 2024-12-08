#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "assert.h"
#include "task_priorities.h"
#include "isotp_links.h"
#include "isotp_link_containers.h"
#include "isotp.h"
#include "mutexes.h"
#include "isotp_tasks.h"

void configure_isotp_link(int index, uint32_t receive_arbitration_id, uint32_t reply_arbitration_id, const char *name) {
    IsoTpLinkContainer *isotp_link_container = &isotp_link_containers[index];
    memset(isotp_link_container, 0, sizeof(IsoTpLinkContainer));
    isotp_link_container->wait_for_isotp_data_sem = xSemaphoreCreateBinary();
    isotp_link_container->recv_buf = calloc(1, ISOTP_BUFSIZE);
    isotp_link_container->send_buf = calloc(1, ISOTP_BUFSIZE);
    isotp_link_container->payload_buf = calloc(1, ISOTP_BUFSIZE);
    isotp_link_container->taskname=name;
    assert(isotp_link_container->recv_buf != NULL);
    assert(isotp_link_container->send_buf != NULL);
    assert(isotp_link_container->payload_buf != NULL);
    isotp_init_link(
        &isotp_link_container->link,
        receive_arbitration_id, reply_arbitration_id,
        isotp_link_container->send_buf, ISOTP_BUFSIZE,
        isotp_link_container->recv_buf, ISOTP_BUFSIZE
    );
    xTaskCreatePinnedToCore(isotp_processing_task, name, 4096, isotp_link_container, ISOTP_TSK_PRIO, NULL, tskNO_AFFINITY);
}

void configure_isotp_links()
{
    // acquire lock
    xSemaphoreTake(isotp_mutex, (TickType_t)100);
    // RX_ID + TX_ID are flipped because this device acts as a "tester" listening for responses from ECUs. the ECU's TX is our RX
    // TODO: make these configurable via j2534 filters
    configure_isotp_link(0, 0x18DA00F1, 0x18DAF100, "CPC");
    configure_isotp_link(1, 0x18DA01F1, 0x18DAF101, "MCM");
    configure_isotp_link(2, 0x18DA03F1, 0x18DAF103, "TCM");
    configure_isotp_link(3, 0x18DA3DF1, 0x18DAF13D, "ACM");
    // free lock
    xSemaphoreGive(isotp_mutex);
}
