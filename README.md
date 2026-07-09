# MPActionRPGSample

UE5 기반 멀티플레이 액션 RPG 미니 샘플 프로젝트입니다.

## Project Goal

이 프로젝트는 완성형 게임 제작이 아니라, 클라이언트 프로그래머 포트폴리오를 목적으로 합니다.

핵심 목표는 다음과 같습니다.

- Dedicated Server 기반 멀티플레이 실행 환경 구성
- Client 2개 접속 검증
- 서버 권위 기반 전투 판정
- HP / 사망 / 리스폰 상태 Replication
- 클라이언트 입력과 서버 판정 흐름 분리
- Replicated State 기반 UI 갱신
- 네트워크 지연 상황에서의 상태 불일치 방지 실험

## Current Scope

1차 목표는 다음 기능까지로 제한합니다.

- 2인 접속
- 이동 동기화
- 기본 공격
- HP Replication
- 사망 / 리스폰
- 몬스터 1종
- 스킬 1개
- HUD 표시
- 테스트 시나리오 문서화

## Out of Scope

초기 버전에서는 아래 기능을 제외합니다.

- 인벤토리
- 장비
- 퀘스트
- 상점
- 로그인 서버
- 매치메이킹
- 대규모 오픈월드
- 복잡한 GAS 구조
- Replication Graph
- Iris

## Dev Log

### Week 1 Day 1 - Project Setup

- UE5 C++ Third Person 기반 프로젝트를 생성했습니다.
- 멀티플레이 액션 RPG 샘플의 1차 목표와 제외 범위를 정리했습니다.
- 초기 단계에서는 전투 구현보다 서버/클라이언트 실행 환경과 네트워크 디버그 표시를 우선 구현하기로 했습니다.