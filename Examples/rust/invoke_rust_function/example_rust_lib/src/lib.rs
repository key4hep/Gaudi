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

struct JobStats {
    events_count: AtomicUsize,
}

impl JobStats {
    pub fn new() -> Self {
        Self {
            events_count: AtomicUsize::new(0),
        }
    }
    pub fn add_event(&self) {
        self.events_count.fetch_add(1, Relaxed);
    }
    pub fn events_count(&self) -> usize {
        self.events_count.load(Relaxed)
    }
}

#[cxx::bridge]
mod ffi {
    extern "Rust" {
        type JobStats;
        fn add_event(&self);
        fn events_count(&self) -> usize;
        fn new_job_stats() -> Box<JobStats>;
    }
}

fn new_job_stats() -> Box<JobStats> {
    Box::new(JobStats::new())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_job_stats() {
        let stats = new_job_stats();
        assert_eq!(stats.events_count(), 0);
        stats.add_event();
        assert_eq!(stats.events_count(), 1);
    }
}
