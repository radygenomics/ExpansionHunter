//
// Expansion Hunter
// Copyright (c) 2016 Illumina, Inc.
//
// Author: Egor Dolzhenko <edolzhenko@illumina.com>
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

#include "graphs/gapless_aligner.h"

#include <iostream>
#include <stdexcept>
#include <vector>

#include "graphs/path_operations.h"

using std::list;
using std::string;
using std::vector;

GraphMapping GaplessAligner::GetBestAlignment(const string& sequence) const {
  const list<string> kmers = ExtractKmersFromAllPositions(sequence, _kmer_len);

  int32_t pos = 0;
  for (const string& kmer : kmers) {
    // Initiate alignment from a unique kmer.
    if (_kmer_index.NumPaths(kmer) == 1) {
      GraphPath kmer_path = _kmer_index.GetPaths(kmer).front();
      return GetBestAlignmentToShortPath(kmer_path, pos, sequence);
    }
    ++pos;
  }
  return GraphMapping();
}

GraphMapping GetBestAlignmentToShortPath(const GraphPath& path,
                                         int32_t start_pos,
                                         const string& sequence) {
  const int32_t start_extension = start_pos;
  const int32_t end_extension = sequence.length() - start_pos - path.Length();
  const list<GraphPath> full_paths =
      path.ExtendBy(start_extension, end_extension);

  GraphMapping best_mapping;
  int32_t max_matches = -1;

  for (const GraphPath& full_path : full_paths) {
    GraphMapping mapping = AlignWithoutGaps(full_path, sequence);
    if (mapping.NumMatches() > max_matches) {
      max_matches = mapping.NumMatches();
      best_mapping = mapping;
    }
  }

  return best_mapping;
}

GraphMapping AlignWithoutGaps(const GraphPath& path, const string& sequence) {
  vector<string> sequence_pieces = SplitByPath(path, sequence);
  vector<Mapping> node_mappings;

  std::shared_ptr<Graph> graph_ptr = path.GraphPtr();
  size_t index = 0;
  for (int32_t node_id : path.NodeIds()) {
    const string node_seq = graph_ptr->NodeSeq(node_id);
    const string sequence_piece = sequence_pieces[index];
    const int32_t ref_start = index == 0 ? path.StartPosition() : 0;
    node_mappings.push_back(
        AlignWithoutGaps(sequence_piece, ref_start, node_seq));
    ++index;
  }

  return GraphMapping(path.NodeIds(), node_mappings);
}

Mapping AlignWithoutGaps(const std::string& query, int32_t ref_start,
                         const std::string& reference) {
  if (reference.length() < ref_start + query.length()) {
    throw std::logic_error("Gapless alignment requires that read " + query +
                           " and " + reference + " have same length.");
  }

  if (query.empty() || reference.empty()) {
    throw std::logic_error("Cannot align empty sequences");
  }

  vector<Operation> operations;
  int32_t previous_run_end = 0;
  int32_t run_length = 0;
  char run_operation = '\0';
  for (size_t index = 0; index != query.length(); ++index) {
    char cur_operation = 'X';
    if (query[index] == reference[ref_start + index]) {
      cur_operation = 'M';
    }

    if (cur_operation == run_operation) {
      ++run_length;
    } else {
      if (run_operation != '\0') {
        const string query_piece = query.substr(previous_run_end, run_length);
        const string reference_piece =
            reference.substr(ref_start + previous_run_end, run_length);
        operations.push_back(
            Operation(run_operation, run_length, query_piece, reference_piece));
      }
      previous_run_end += run_length;
      run_length = 1;
      run_operation = cur_operation;
    }
  }

  const string query_piece = query.substr(previous_run_end, run_length);
  const string reference_piece =
      reference.substr(ref_start + previous_run_end, run_length);
  operations.push_back(
      Operation(run_operation, run_length, query_piece, reference_piece));

  return Mapping(ref_start, operations);
}

list<string> ExtractKmersFromAllPositions(const std::string& sequence,
                                          int32_t kmer_len) {
  list<string> kmers;
  for (size_t pos = 0; pos + kmer_len <= sequence.length(); ++pos) {
    kmers.push_back(sequence.substr(pos, kmer_len));
  }
  return kmers;
}