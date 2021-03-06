//
// Expansion Hunter
// Copyright (c) 2016 Illumina, Inc.
//
// Author: Egor Dolzhenko <edolzhenko@illumina.com>,
//         Mitch Bekritsky <mbekritsky@illumina.com>, Richard Shaw
// Concept: Michael Eberle <meberle@illumina.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#pragma once

#include <map>
#include <vector>
#include <string>

#include "common/genomic_region.h"
#include "include/read_group.h"
#include "genotyping/short_repeat_genotyper.h"

class RegionFindings {
 public:
  std::string region_id;
  Region region;
  int num_anchored_irrs;
  int num_unaligned_irrs;
  int num_irrs;
  std::vector<RepeatReadGroup> read_groups;
  std::vector<RepeatAlign> rep_aligns;
  std::vector<RepeatAlign> flanking_repaligns;
  std::vector<int> offtarget_irr_counts;
  RepeatGenotype genotype;
};
