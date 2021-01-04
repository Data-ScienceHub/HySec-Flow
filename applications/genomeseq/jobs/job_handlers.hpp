#ifndef D1147A7C_FF42_4F4E_9F40_AAE8053CFAA5
#define D1147A7C_FF42_4F4E_9F40_AAE8053CFAA5
#include "dispatch_job.hpp"
#include "partition_job.hpp"
#include "spdlog/spdlog.h"

void ScheduleIndexJobs(std::string index_id, int32_t partitions, std::string client_id,
                       std::shared_ptr<tasker::Driver> driver,
                       std::string src_file,
                       std::shared_ptr<std::function<void(int32_t, std::string, std::string)>> create_index) {
    std::shared_ptr<tasker::Jobs> indexing_jobs = std::make_shared<tasker::Jobs>(
        std::make_shared<std::function<void(int32_t, int32_t, std::string)>>(
            [=](int32_t failed_count, int32_t failed_code, std::string msg) {
                // on all jobs done
                spdlog::info("All indexing jobs have been completed.");
                if (failed_count != 0) {
                    spdlog::warn("{} of the indexing jobs has failed...", failed_count);
                } else {
                    spdlog::info("Sending indexing response to the client...");
                    driver->SendToClient(client_id, tasker::GetCommand(tasker::Commands::MESSAGE),
                                         "Indexing completed and assigned ID " + index_id);

                    // adding index to the DB
                    spdlog::info("Calling db callback...");
                    (*create_index)(partitions, index_id, src_file);
                }
            }),
        std::make_shared<std::function<void(std::string, int32_t, std::string, int32_t, int32_t)>>(
            [=](std::string job_id, int32_t code, std::string msg, int32_t failed_jobs, int32_t completed_jobs) {
                // on one of the jobs done
                if (code != 0 && failed_jobs == 1) {
                    // this is the first failed job, notify client
                    std::string error_msg = "Indexing failed for job " + job_id + ". " + msg;
                    driver->SendToClient(client_id, tasker::GetCommand(tasker::Commands::MESSAGE), error_msg);
                }
                spdlog::info("Index {} completed {}/{}, Failures : {}", index_id, completed_jobs, partitions, failed_jobs);
            }),
        driver);

    spdlog::info("Creating indexing jobs for {} partitions.", partitions);
    for (int32_t idx = 0; idx < partitions; idx++) {
        std::string idx_src = index_id + "/mref-" + std::to_string(idx + 1) + ".fa";
        std::string job_id = gen_random(16);
        std::shared_ptr<IndexJob> idx_job = std::make_shared<IndexJob>(
            idx_src, job_id,
            idx,
            client_id,
            driver);
        spdlog::info("Created index job for partition {}", idx + 1);
        indexing_jobs->AddJob(idx_job);
        spdlog::info("Added job for partition {}", idx + 1);
    }
    indexing_jobs->Execute();
}

void HandleIndex(std::string msg, std::string client_id,
                 std::shared_ptr<tasker::Driver> driver,
                 std::shared_ptr<std::function<void(int32_t, std::string, std::string)>> create_index) {
    std::string index_id = "index-" + gen_random(8);
    std::string job_id = gen_random(16);

    spdlog::info("Creating new index {}", index_id);
    spdlog::info("Handling parition commnad...");

    std::shared_ptr<PartitionJob> prt_job = std::make_shared<PartitionJob>(msg, job_id, client_id, index_id,
                                                                           driver);

    prt_job->OnComplete(std::make_shared<std::function<void(std::string, int32_t, std::string)>>([=](std::string job_id,
                                                                                                     int32_t code, std::string msg) {
        if (code != 0) {
            spdlog::info("Parition job {} has failed. Not scheduling indexing");
            // reporting the error to the client
            driver->SendToClient(client_id, tasker::GetCommand(tasker::Commands::MESSAGE), msg);
        } else {
            //shedule indexing
            ScheduleIndexJobs(index_id, prt_job->GetCommand()->GetPartitions(),
                              client_id, driver, prt_job->GetCommand()->GetSrcFile(), create_index);
        }
    }));
    spdlog::info("Created parition job object");
    driver->GetExecutor()->AddJob(prt_job);
}

void HandleDispatch(std::string msg, std::string client_id,
                    std::shared_ptr<tasker::Driver> driver) {
    spdlog::info("Handling dispatch commnad...");
    std::string job_id = gen_random(16);
    std::shared_ptr<DispatchJob> dsp_job = std::make_shared<DispatchJob>(msg, job_id, client_id,
                                                                         driver);
    spdlog::info("Created dispatch job object");
    driver->GetExecutor()->AddJob(dsp_job);
}
#endif /* D1147A7C_FF42_4F4E_9F40_AAE8053CFAA5 */