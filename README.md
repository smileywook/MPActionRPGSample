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

### Week 1 Day 2 - PIE Multiplayer Test

- Unreal Editor의 Play In Editor 멀티플레이 옵션을 확인했습니다.
- Number of Players를 2로 설정하고 Listen Server 모드에서 두 플레이어가 같은 맵에 접속하는 것을 확인했습니다.
- Play As Client 모드에서 백그라운드 서버와 2개의 클라이언트 창이 실행되는 흐름을 확인했습니다.
- Listen Server는 서버 역할을 겸하는 플레이어 창이 존재하고, Play As Client는 Dedicated Server에 가까운 구조로 클라이언트 창만 표시된다는 차이를 정리했습니다.
- 이후 네트워크 디버그 UI에서 NetMode / LocalRole / RemoteRole을 표시하기로 했습니다.

### Week 1 Day 3 - Gameplay Framework Base Classes

- 멀티플레이 샘플에서 사용할 기본 Gameplay Framework 클래스를 생성했습니다.
- MPGameMode, MPGameState, MPPlayerState, MPPlayerController를 추가했습니다.
- MPGameMode에서 GameStateClass, PlayerControllerClass, PlayerStateClass, DefaultPawnClass를 명시적으로 지정했습니다.
- GameMode는 서버 전용 게임 규칙, GameState는 전체 게임 상태, PlayerState는 플레이어별 상태, PlayerController는 입력과 UI 소유 역할로 분리하기로 했습니다.
- PIE 멀티플레이 실행에서 기존 Third Person Character가 정상적으로 스폰되고 이동 동기화되는 것을 확인했습니다.