// #####################################################################################
// # (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      #
// #                                                                                   #
// # This software is distributed under the terms of the Apache version 2 licence,     #
// # copied verbatim in the file "LICENSE".                                            #
// #                                                                                   #
// # In applying this licence, CERN does not waive the privileges and immunities       #
// # granted to it by virtue of its status as an Intergovernmental Organization        #
// # or submit itself to any jurisdiction.                                             #
// #####################################################################################
use std::sync::atomic::{AtomicUsize, Ordering::Relaxed};

#[cxx::bridge]
mod ffi {
    extern "Rust" {
        type JobStats;
        fn init_job_stats() -> Box<JobStats>;
        fn increment(&self);
        fn events_count(&self) -> usize;
    }
}

struct JobStats {
    events_count: AtomicUsize,
}

fn init_job_stats() -> Box<JobStats> {
    Box::new(JobStats::new())
}

impl JobStats {
    pub fn new() -> Self {
        Self {
            events_count: AtomicUsize::new(0),
        }
    }
    pub fn increment(&self) {
        self.events_count.fetch_add(1, Relaxed);
    }
    pub fn events_count(&self) -> usize {
        self.events_count.load(Relaxed)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_job_stats() {
        let stats = init_job_stats();
        assert_eq!(stats.events_count(), 0);
        stats.increment();
        assert_eq!(stats.events_count(), 1);
    }
}
