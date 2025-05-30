#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <map>
#include <queue>

#include "utils/cache/lru.h"
#include "database/init.h"
#include "database/dao/pcu_relay_cnt/pcu_relay_cnt.h"
#include "fcgi/fcgi.h"
#include "relay/relay.h"
#include "utils/priority_queue/priority_queue.h"
#include "can/can.h"

#include "mode/singleton.h"
#include "mode/composite.h"
#include "mode/template.h"

using namespace std;


class Solution
{
    public:
        void merge(vector<int>& nums1, int m, vector<int>& nums2, int n)
        {
            int i = m - 1;
            int j = n - 1;
            int k = m + n - 1;
            while (i >= 0 && j>=0)
            {
                if (nums1[i] > nums2[j])
                {
                    nums1[k--] = nums1[i--];
                }
                else
                {
                    nums1[k--] = nums2[j--];
                }
            }
            while (j >= 0) {
                nums1[k--] = nums2[j--];
            }
        }
        int removeElement(vector<int>& nums, int val) {
            int size = nums.size();
            vector<int> nums2;
            int num = 0;
            for (int i = 0; i < size; i++)
            {
                if (nums[i] != val)
                {
                    nums2.push_back(nums[i]);
                    num++;
                }
            }
            nums.resize(num);
            int k = 0;
            for (int j : nums2)
            {
                nums[k++] = j;
                d_log("j:%d", j);
            }
            return num;
        }
        int removeDuplicates(vector<int>& nums) {
            if (nums.empty()) return 0;
            int k = 1;
            int count = 1;
            for (int i = 1; i < nums.size(); i++)
            {
                if (nums[i] == nums[k-1])
                {
                    count++;
                }
                else
                {
                    count = 1;
                }

                if (count <=2)
                {
                    nums[k++] = nums[i];
                }
            }
            nums.resize(k);
            return k;
        }

        int majorityElement(vector<int>& nums) {
            unordered_map<int, int>counts;
            int majority = 0, cnt = 0;
            for (int i : nums)
            {
                counts[i]++;
                if (counts[i] > cnt)
                {
                    majority = i;
                    cnt = counts[i];
                }
            }
            return majority;
        }

        void rotate(vector<int>& nums, int k) {
            reverse(nums.begin(), nums.end());
            reverse(nums.begin(), nums.begin() + k - 1);
            reverse(nums.begin() + k, nums.end());
        }

        int maxProfit(vector<int>& prices) {
            int ans = 0;
            for (uint8_t i = 0; i < prices.size() - 1; i++)
            {
                if (prices[i] < prices[i + 1])
                {
                    ans += (prices[i + 1] - prices[i]);
                }
            }
            return ans;
        }

        bool canJump(vector<int>& nums) {
            int k = 0;
            for(int i = 0; i < nums.size(); i++) {
                if(k < i) return false;
                k = max(k, i + nums[i]);
            }
            return true;
        }

};

int32_t can_func(struct can_frame frame)
{
    d_log("frame.id:[%08x] [%02x %02x %02x %02x %02x %02x %02x %02x ]", frame.can_id,\
    frame.data[0], frame.data[1], frame.data[2], frame.data[3],\
    frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
}

void *func1(void *arg)
{
    pq_t *pq = (pq_t *)arg;
    struct can_frame frame
    {.can_id = 0x060F8039,
     .can_dlc = 8,
     .data = {0x41, 0xF0, 0x00, 0x01, 0x3E, 0xC8, 0x00, 0x00}
    };
    can_msg_t msg  = {.frame = frame, .callback = can_func};

    pv_t item1 = {._priority = 11, ._value = (void *)&msg};
    pv_t item2 = {._priority = 10, ._value = NULL};
    pv_t item3 = {._priority = 9, ._value = NULL};
    pv_t item4 = {._priority = 8, ._value = NULL};
    pv_t item5 = {._priority = 7, ._value = NULL};
    pv_t item6 = {._priority = 6, ._value = NULL};
    pv_t item7 = {._priority = 5, ._value = NULL};
    pv_t item8 = {._priority = 4, ._value = NULL};
    pv_t item9 = {._priority = 3, ._value = NULL};
    pv_t item10 = {._priority = 2, ._value = NULL};
    pv_t item11 = {._priority = 1, ._value = NULL};

    while (1)
    {
        priority_queue_push(pq, item1);
        usleep(1000);
        priority_queue_push(pq, item2);
        usleep(1000);
        priority_queue_push(pq, item3);
        usleep(1000);
        priority_queue_push(pq, item4);
        usleep(1000);
        priority_queue_push(pq, item5);
        usleep(1000);
        priority_queue_push(pq, item6);
        usleep(1000);
        priority_queue_push(pq, item7);
        usleep(1000);
        priority_queue_push(pq, item8);
        usleep(1000);
        priority_queue_push(pq, item9);
        usleep(1000);
        priority_queue_push(pq, item10);
        usleep(1000);
        priority_queue_push(pq, item11);
        usleep(1000);
    }
}

void *func2(void *arg)
{
    pq_t *pq = (pq_t *)arg;

    while (1)
    {
        // pv_t item = pq_top(pq);
        pv_t item = {0};
        priority_queue_pop(pq, &item);
        if (item._value != NULL)
        {
            can_msg_t *tmp = (can_msg_t *)item._value;
            if (NULL != tmp->callback)
            {
                d_log("Top key: %d", item._priority);
                tmp->callback(tmp->frame);
            }
            else
            {
                d_log("callback is NULL");
            }

            // if (item._icd._dele)
            // {
            //     item._icd._dele(item._value);
            // }
            // else
            // {
            //     d_log("icd._dele is NULL");
            // }
        }
        else
        {
            d_log("Top key: %d _value is NULL", item._priority);
        }

        usleep(1000);
    }
}

int main(void)
{
    pq_t *pq = pq_init(10, max_heap_compare);


    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, func1, (void *)pq);
    pthread_create(&tid2, NULL, func2, (void *)pq);

    while(1)
    {
        sleep(1);
    }

    return 0;
}

